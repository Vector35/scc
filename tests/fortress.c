// Copyright (c) 2011-2012 Rusty Wagner
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

typedef struct Property
{
	char address[128];
	char name[32];
	void (*display)(struct Property* prop);

	uint32_t askingPrice;
	uint32_t sellPrice;

	uint32_t squareFeet;
	uint32_t bedrooms;
	uint32_t bathroomsTimes2;

	struct Property* prev;
	struct Property* next;
} Property;


Property* g_forSale = NULL;
Property* g_sold = NULL;


int readUntil(char* buffer, size_t len, char sentinal)
{
	char* ptr = buffer;
	for (size_t i = 0;; i++)
	{
		char ch;
		if (read(0, &ch, 1) != 1)
			return i;
		if (ch == sentinal)
			return i;
		if (i >= len)
			return i;
		*(ptr++) = ch;
	}
	return len;
}


void displayCommercialProperty(Property* prop)
{
	printf("Commercial property: %s\n", prop->name);
	printf("Address:\n%s\n\n", prop->address);
	printf("Square footage: %d\n", prop->squareFeet);
}


void displayResidentialProperty(Property* prop)
{
	printf("Residential property: %s\n", prop->name);
	printf("Address:\n%s\n\n", prop->address);
	printf("Square footage: %d\n", prop->squareFeet);
	printf("Number of bedrooms: %d\n", prop->bedrooms);

	printf("Number of bathrooms: %d", prop->bathroomsTimes2 / 2);
	if (prop->bathroomsTimes2 & 1)
		puts(".5");
	puts("\n");
}


Property* findPropertyByIndex(uint32_t desiredId)
{
	uint32_t id = 1;
	Property* prop;
	for (prop = g_forSale; prop; prop = prop->next, id++)
	{
		if (desiredId == id)
			break;
	}
	return prop;
}


void showProperties()
{
	if (!g_forSale)
	{
		puts("No properties are currently for sale\n");
		return;
	}

	puts("    Property name                      Asking price\n");
	uint32_t id = 1;
	for (Property* prop = g_forSale; prop; prop = prop->next, id++)
		printf("%2d) %-32s   %12d\n", id, prop->name, prop->askingPrice);
}


void showPropertyDetails()
{
	puts("Property number to view: ");

	char buffer[16];
	int result = readUntil(buffer, 15, '\n');
	buffer[result] = 0;
	uint32_t id = atoi(buffer);
	puts("\n");

	Property* prop = findPropertyByIndex(id);
	if (!prop)
	{
		puts("Invalid property number\n");
		return;
	}

	prop->display(prop);
}


void addProperty()
{
	Property* prop = (Property*)malloc(sizeof(Property));
	int line2Offset;
	char buffer[128];
	memset(prop, 0, sizeof(Property));

	puts("Property type:\n");
	puts("1) Residential\n");
	puts("2) Commercial\n");
	puts("Selection: ");
	int result = readUntil(buffer, 16, '\n');
	buffer[result] = 0;
	bool commercial = false;
	if (atoi(buffer) == 2)
		commercial = true;

	prop->display = commercial ? displayCommercialProperty : displayResidentialProperty;

	puts("\nProperty name: ");

	result = readUntil(buffer, 128, '\n');
	if (result > 32)
	{
		puts("Name too long\n");
		free(prop);
		return;
	}
	buffer[result] = 0;
	strncpy(prop->name, buffer, 32);

	puts("Address line 1: ");

	result = readUntil(buffer, 128, '\n');
	if (result >= 128)
	{
		puts("Address too long\n");
		free(prop);
		return;
	}

	buffer[result] = '\n';

	puts("Address line 2: ");

	line2Offset = result + 1;
	int restLen = 127 - line2Offset;
	result = readUntil((char*)buffer + line2Offset, restLen, '\n');
	buffer[line2Offset + result] = 0;

	strncpy(prop->address, buffer, 128);

	puts("Square footage: ");
	result = readUntil(buffer, 16, '\n');
	buffer[result] = 0;
	prop->squareFeet = atoi(buffer);

	if (!commercial)
	{
		puts("Number of bedrooms: ");
		result = readUntil(buffer, 16, '\n');
		buffer[result] = 0;
		prop->bedrooms = atoi(buffer);

		puts("Number of bathrooms:\n");
		puts("1) 1 bath\n");
		puts("2) 1.5 bath\n");
		puts("3) 2 bath\n");
		puts("4) 2.5 bath\n");
		puts("5) 3 bath\n");
		puts("6) 3.5 bath\n");
		puts("7) 4 bath\n");
		puts("Selection: ");

		result = readUntil(buffer, 16, '\n');
		buffer[result] = 0;
		prop->bathroomsTimes2 = atoi(buffer) + 1;
	}

	puts("\nAsking price: ");
	result = readUntil(buffer, 16, '\n');
	buffer[result] = 0;
	prop->askingPrice = atoi(buffer);

	prop->prev = NULL;
	prop->next = g_forSale;
	if (g_forSale)
		g_forSale->prev = prop;
	g_forSale = prop;
}


void changeAskingPrice()
{
	puts("Property number to change: ");

	char buffer[16];
	int result = readUntil(buffer, 15, '\n');
	buffer[result] = 0;
	uint32_t id = atoi(buffer);

	Property* prop = findPropertyByIndex(id);
	if (!prop)
	{
		puts("Invalid property number\n");
		return;
	}

	puts("New asking price: ");
	result = readUntil(buffer, 15, '\n');
	buffer[result] = 0;
	prop->askingPrice = atoi(buffer);
}


void newSale()
{
	puts("Property number to sell: ");

	char buffer[16];
	int result = readUntil(buffer, 15, '\n');
	buffer[result] = 0;
	uint32_t id = atoi(buffer);

	Property* prop = findPropertyByIndex(id);
	if (!prop)
	{
		puts("Invalid property number\n");
		return;
	}

	puts("Selling price: ");
	result = readUntil(buffer, 15, '\n');
	buffer[result] = 0;
	prop->sellPrice = atoi(buffer);

	if (prop->prev != NULL)
		prop->prev->next = prop->next;
	else
		g_forSale = prop->next;

	if (prop->next != NULL)
		prop->next->prev = prop->prev;

	prop->prev = NULL;
	prop->next = g_sold;
	if (g_sold != NULL)
		g_sold->prev = prop;
	g_sold = prop;
}


void showTransactions()
{
	if (!g_sold)
	{
		puts("No properties have been sold\n");
		return;
	}

	puts("    Property name                      Asking price  Selling price\n");
	uint32_t id = 1;
	for (Property* prop = g_sold; prop; prop = prop->next, id++)
		printf("%2d) %-32s   %12d   %12d\n", id, prop->name, prop->askingPrice, prop->sellPrice);
}


int main()
{
	puts("FORTRESS Property Management Server\n");

	while (true)
	{
		puts("\nMain menu:\n");
		puts("1) Show properties for sale\n");
		puts("2) Show property details\n");
		puts("3) Put a property up for sale\n");
		puts("4) Change asking price\n");
		puts("5) Log a new sale\n");
		puts("6) Show transaction report\n");
		puts("7) Log off\n");
		puts("Selection: ");

		char str[8];
		int result = readUntil(str, 7, '\n');
		str[result] = 0;
		int option = atoi(str);

		puts("\n");

		switch (option)
		{
		case 1:
			showProperties();
			break;
		case 2:
			showPropertyDetails();
			break;
		case 3:
			addProperty();
			break;
		case 4:
			changeAskingPrice();
			break;
		case 5:
			newSale();
			break;
		case 6:
			showTransactions();
			break;
		case 7:
			return 0;
		default:
			break;
		}
	}
}
