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

#include "RegisterClass.h"
#include "CodeBlock.h"

using namespace std;


RegisterClassDef::RegisterClassDef(RegisterClassType type, uint32_t sizeFlags,
    const string& matchName, const string& regClass, CodeBlock* fixed) :
    m_classType(type),
    m_sizeFlags(sizeFlags), m_matchName(matchName), m_regClass(regClass), m_fixed(fixed)
{}


RegisterClassDef::RegisterClassDef(
    uint32_t sizeFlags, const string& matchName, const string& lowClass, const string& highClass) :
    m_classType(REGCLASS_LARGE),
    m_sizeFlags(sizeFlags), m_matchName(matchName), m_regClass(lowClass), m_highClass(highClass)
{}
