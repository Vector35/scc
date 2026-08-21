// Copyright 2026 Vector 35 Inc.
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

struct a
{
	union
	{
		int b;
		int c;
	} d;
};

int read_nested_union_member(struct a* e)
{
	return e->d.b;
}

void write_nested_union_member(struct a* e, int value)
{
	e->d.b = value;
}

int* address_of_nested_union_member(struct a* e)
{
	return &e->d.b;
}

struct inner
{
	int padding;
	int value;
};

struct outer
{
	int padding;
	struct inner inner;
};

struct root
{
	int padding;
	struct outer outer;
};

struct array_inner
{
	int padding;
	int values[3];
};

struct array_outer
{
	int padding;
	struct array_inner inner;
};

struct pointer_inner
{
	int* value;
};

struct pointer_outer
{
	int padding;
	struct pointer_inner inner;
};

typedef int matrix_row[3];

int read_deeply_nested_member(struct root* value)
{
	return value->outer.inner.value;
}

int read_nested_member_through_dereference(struct root* value)
{
	return (*value).outer.inner.value;
}

int read_nested_member_from_array(struct root* values)
{
	return values[1].outer.inner.value;
}

int read_nested_array_member(struct array_outer* value, int index)
{
	return value->inner.values[index];
}

void write_nested_array_member(struct array_outer* value, int index, int item)
{
	value->inner.values[index] = item;
}

int* address_of_nested_array_member(struct array_outer* value, int index)
{
	return &value->inner.values[index];
}

int pre_increment_nested_member(struct root* value)
{
	return ++value->outer.inner.value;
}

int post_increment_nested_member(struct root* value)
{
	return value->outer.inner.value++;
}

int pre_decrement_nested_member(struct root* value)
{
	return --value->outer.inner.value;
}

int post_decrement_nested_member(struct root* value)
{
	return value->outer.inner.value--;
}

struct root* count_root_access(struct root* value, int* count)
{
	(*count)++;
	return value;
}

int post_increment_nested_member_once(struct root* value, int* count)
{
	return count_root_access(value, count)->outer.inner.value++;
}

int* pre_increment_nested_pointer(struct pointer_outer* value)
{
	return ++value->inner.value;
}

int* post_decrement_nested_pointer(struct pointer_outer* value)
{
	return value->inner.value--;
}

int read_matrix_member(matrix_row* matrix, int row, int column)
{
	return matrix[row][column];
}

void write_matrix_member(matrix_row* matrix, int row, int column, int value)
{
	matrix[row][column] = value;
}

int* address_of_matrix_member(matrix_row* matrix, int row, int column)
{
	return &matrix[row][column];
}

int read_first_matrix_row(matrix_row* matrix, int column)
{
	return (*matrix)[column];
}

void write_first_matrix_row(matrix_row* matrix, int column, int value)
{
	(*matrix)[column] = value;
}

int main()
{
	struct a e;
	struct array_outer array_value;
	struct pointer_outer pointer_value;
	struct root direct_root;
	struct root roots[2];
	int* address;
	int count;
	int index;
	matrix_row matrix[2];
	int pointer_values[3];

	e.d.b = 0x12345678;
	if (read_nested_union_member(&e) != 0x12345678)
		return 1;

	write_nested_union_member(&e, 0x76543210);
	if (e.d.b != 0x76543210)
		return 2;

	address = address_of_nested_union_member(&e);
	*address = 0x13572468;
	if (e.d.b != 0x13572468)
		return 3;

	roots[0].outer.inner.value = 0x24681357;
	if (read_deeply_nested_member(&roots[0]) != 0x24681357)
		return 4;
	if (read_nested_member_through_dereference(&roots[0]) != 0x24681357)
		return 5;

	roots[1].outer.inner.value = 0x10293847;
	if (read_nested_member_from_array(roots) != 0x10293847)
		return 6;

	array_value.inner.values[1] = 0x11223344;
	if (read_nested_array_member(&array_value, 1) != 0x11223344)
		return 7;

	write_nested_array_member(&array_value, 2, 0x55667788);
	if (array_value.inner.values[2] != 0x55667788)
		return 8;

	address = address_of_nested_array_member(&array_value, 0);
	*address = 0x12344321;
	if (array_value.inner.values[0] != 0x12344321)
		return 9;

	roots[0].outer.inner.value = 10;
	if (pre_increment_nested_member(&roots[0]) != 11)
		return 10;
	if (post_increment_nested_member(&roots[0]) != 11)
		return 11;
	if (roots[0].outer.inner.value != 12)
		return 12;
	if (pre_decrement_nested_member(&roots[0]) != 11)
		return 13;
	if (post_decrement_nested_member(&roots[0]) != 11)
		return 14;
	if (roots[0].outer.inner.value != 10)
		return 15;

	direct_root.outer.inner.value = 20;
	if (++direct_root.outer.inner.value != 21)
		return 16;
	if (direct_root.outer.inner.value-- != 21)
		return 17;
	if (direct_root.outer.inner.value != 20)
		return 18;

	array_value.inner.values[0] = 30;
	if (++array_value.inner.values[0] != 31)
		return 19;
	if (array_value.inner.values[0]-- != 31)
		return 20;
	if (array_value.inner.values[0] != 30)
		return 21;

	address = &array_value.inner.values[0];
	if (++*address != 31)
		return 22;
	if ((*address)-- != 31)
		return 23;
	if (*address != 30)
		return 24;
	if (&*address != address)
		return 25;

	count = 0;
	roots[0].outer.inner.value = 40;
	if (post_increment_nested_member_once(&roots[0], &count) != 40)
		return 26;
	if (roots[0].outer.inner.value != 41)
		return 27;
	if (count != 1)
		return 28;

	index = 0;
	if (array_value.inner.values[index++]++ != 30)
		return 29;
	if (index != 1)
		return 30;
	if (array_value.inner.values[0] != 31)
		return 31;

	pointer_value.inner.value = &pointer_values[0];
	if (pre_increment_nested_pointer(&pointer_value) != &pointer_values[1])
		return 32;
	if (post_decrement_nested_pointer(&pointer_value) != &pointer_values[1])
		return 33;
	if (pointer_value.inner.value != &pointer_values[0])
		return 34;

	write_matrix_member(matrix, 1, 2, 17);
	if (read_matrix_member(matrix, 1, 2) != 17)
		return 35;
	if (matrix[1][2] != 17)
		return 36;
	address = address_of_matrix_member(matrix, 1, 0);
	*address = 19;
	if (matrix[1][0] != 19)
		return 37;
	matrix[0][1] = 23;
	if (read_first_matrix_row(matrix, 1) != 23)
		return 38;
	if (&(*matrix)[1] != &matrix[0][1])
		return 39;
	write_first_matrix_row(matrix, 2, 29);
	if (matrix[0][2] != 29)
		return 40;

	return 0;
}
