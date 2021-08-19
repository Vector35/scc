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

#ifndef __REFCOUNTOBJECT_H__
#define __REFCOUNTOBJECT_H__

#ifdef WIN32
	#include <windows.h>
#endif
#include <stddef.h>


class RefCountObject
{
 public:
	int m_refs;
	RefCountObject() : m_refs(0) {}
	virtual ~RefCountObject() {}

	void AddRef()
	{
#ifdef WIN32
		InterlockedIncrement((LONG*)&m_refs);
#else
		__sync_fetch_and_add(&m_refs, 1);
#endif
	}

	void Release()
	{
#ifdef WIN32
		if (InterlockedDecrement((LONG*)&m_refs) == 0)
			delete this;
#else
		if (__sync_fetch_and_add(&m_refs, -1) == 1)
			delete this;
#endif
	}
};


template <class T>
class Ref
{
	T* m_obj;

 public:
	Ref<T>() : m_obj(NULL) {}

	Ref<T>(T* obj) : m_obj(obj)
	{
		if (m_obj)
			m_obj->AddRef();
	}

	Ref<T>(const Ref<T>& obj) : m_obj(obj.m_obj)
	{
		if (m_obj)
			m_obj->AddRef();
	}

	~Ref<T>()
	{
		if (m_obj)
			m_obj->Release();
	}

	Ref<T>& operator=(const Ref<T>& obj)
	{
		T* oldObj = m_obj;
		m_obj = obj.m_obj;
		if (m_obj)
			m_obj->AddRef();
		if (oldObj)
			oldObj->Release();
		return *this;
	}

	Ref<T>& operator=(T* obj)
	{
		T* oldObj = m_obj;
		m_obj = obj;
		if (m_obj)
			m_obj->AddRef();
		if (oldObj)
			oldObj->Release();
		return *this;
	}

	operator T*() const { return m_obj; }

	T* operator->() const { return m_obj; }

	T& operator*() const { return *m_obj; }

	bool operator!() const { return m_obj == NULL; }

	T* GetPtr() const { return m_obj; }
};


#endif
