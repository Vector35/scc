// Copyright (c) 2014 Rusty Wagner
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

#ifndef __REGISTERCLASS_H__
#define __REGISTERCLASS_H__

#include "RefCountObject.h"
#include <string>


enum RegisterClassType
{
	REGCLASS_NORMAL,
	REGCLASS_LARGE,
	REGCLASS_TEMP
};

enum RegisterType
{
	REGTYPE_INT8,
	REGTYPE_INT16,
	REGTYPE_INT32,
	REGTYPE_INT64,
	REGTYPE_FLOAT
};

class RegisterClass: public RefCountObject
{
	RegisterClassType m_classType;
	RegisterType m_regType;
	std::string m_matchName, m_regClass, m_highClass;

public:
	RegisterClass(const std::string& matchName);
	RegisterClass(const std::string& matchName, RegisterType type, const std::string& regClass);
	RegisterClass(const std::string& matchName, RegisterType type, const std::string& lowClass, const std::string& highClass);

	RegisterClassType GetClassType() const { return m_classType; }
	RegisterType GetRegisterType() const { return m_regType; }

	const std::string& GetMatchName() const { return m_matchName; }
	const std::string& GetRegisterClassName() const { return m_regClass; }
	const std::string& GetLowRegisterClassName() const { return m_regClass; }
	const std::string& GetHighRegisterClassName() const { return m_highClass; }
};


#endif

