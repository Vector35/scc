#ifndef __REFCOUNTOBJECT_H__
#define __REFCOUNTOBJECT_H__

#ifdef WIN32
#include <windows.h>
#endif
#include <atomic>
#include <stddef.h>


class RefCountObject
{
public:
	std::atomic<int> m_refs;
	RefCountObject(): m_refs(0) {}
	virtual ~RefCountObject() {}

	void AddRef()
	{
		m_refs.fetch_add(1);
	}

	void Release()
	{
		if (m_refs.fetch_sub(1) == 1)
			delete this;
	}
};


template <class T>
class Ref
{
	T* m_obj;

public:
	Ref<T>(): m_obj(NULL)
	{
	}

	Ref<T>(T* obj): m_obj(obj)
	{
		if (m_obj)
			m_obj->AddRef();
	}

	Ref<T>(const Ref<T>& obj): m_obj(obj.m_obj)
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

	operator T*() const
	{
		return m_obj;
	}

	T* operator->() const
	{
		return m_obj;
	}

	T& operator*() const
	{
		return *m_obj;
	}

	bool operator!() const
	{
		return m_obj == NULL;
	}

	T* GetPtr() const
	{
		return m_obj;
	}
};


#endif

