/**
 * qcy::Vec: My own implementation of std::vector, All the interfaces' function are the same as std::vector.
 * Adding the cheatyClear() function to qcy::Vec is intended to optimize performence of this project.
 * The time complexity of cheatyClear() could achieve O(1) and it can works correctly in this project.
 */
#ifndef __VEC__H
#define __VEC__H

#include <memory>
#include <iostream>	

#define CLEAR_CHEAT

namespace qcy
{
	
	template <typename T> class Vec
	{
	public:
		typedef T elem_type;
		typedef T* iterator;

		/* All the interfaces' function are the same as std::vector except cheatyClear() */
#ifndef CLEAR_CHEAT
		Vec() : elements(nullptr), firstFree(nullptr), cap(nullptr) {};
#else 	
		Vec() : elements(nullptr), firstFree(nullptr), cap(nullptr), firstNotConstruct(nullptr) {};
#endif 	

		Vec(const Vec& _vec);
		Vec& operator = (const Vec& _vec);

		Vec(Vec&& _vec);
		Vec& operator = (Vec&& _vec);

		~Vec();

		void resize(size_t n);
		void resize(size_t n, const T& value);

		void push_back(const T& _data);
		void clear();
		elem_type& operator [] (int i) { return elements[i]; };
		elem_type& front() const { return *elements; };
		elem_type& back() const { return *(elements + size() - 1); };

#ifdef CLEAR_CHEAT
		// This interface's caller should know every detail of this class.
		void cheatyClear() { firstFree = elements; };
#endif 

		elem_type* begin() const { return elements; };
		elem_type* end() const { return firstFree; };
		size_t size() const { return firstFree - elements; };
		size_t capacity() const { return cap - elements; };
		bool empty() const { return elements == firstFree; };
		

	// private
	public:
		std::allocator<T> alloc;
		elem_type* elements;
		elem_type* firstFree;
		elem_type* cap;

#ifdef CLEAR_CHEAT
		elem_type* firstNotConstruct;
#endif
		void reallocate();
		void check();
		void check_n_cap(size_t n);
		void Free();
	};

	template<typename T> 
	Vec<T>::Vec(const Vec& _vec)
	{
		elements = alloc.allocate(_vec.capacity());
		firstFree = std::uninitialized_copy(_vec.elements, _vec.firstFree, elements);
		cap = elements + _vec.capacity();
		
	}

	template<typename T>
	Vec<T>& Vec<T>::operator = (const Vec& _vec)
	{
		size_t newCapacity = _vec.capacity();
		T* newElements = alloc.allocate(newCapacity);
		T* newFirstFree = std::uninitialized_copy(_vec.elements, _vec.firstFree, newElements);
		T* newCap = newElements + newCapacity;
		Free();
		elements = newElements;
		firstFree = newFirstFree;
		cap = newCap;
		return *this;
	}

	template <typename T>
	Vec<T>::Vec(Vec&& _vec) : elements(_vec.elements),firstFree(_vec.firstFree),cap(_vec.cap)
	{
		std::cout << "move constructor" << std::endl;
		_vec.elements = _vec.firstFree = _vec.cap = nullptr;
	}

	template <typename T>
	Vec<T>& Vec<T>::operator = (Vec&& _vec)
	{
		std::cout << "move operator = " << std::endl;
		T* newElements = _vec.elements;
		T* newFirstFree = _vec.firstFree;
		T* newCap = _vec.cap;

		Free();
		_vec.elements = _vec.firstFree = _vec.cap = nullptr;
		elements = newElements;
		firstFree = newFirstFree;
		cap = newCap;
		return *this;
	}

	template<typename T>
	Vec<T>::~Vec()
	{
		Free();
	}

	template<typename T>
	void Vec<T>::resize(size_t n)
	{
		if (n < size())
		{
			T* flag = &elements[n];
			while (firstFree != flag)
			{
				alloc.destroy(--firstFree);
			}
		}
		else if (n > size())
		{
			check_n_cap(n);
			size_t numRest = n - size();
			while (numRest != 0)
			{
				alloc.construct(firstFree++);
				numRest--;
			}
		}
	}

	template <typename T>
	void Vec<T>::resize(size_t n, const T& value)
	{
		if (n < size())
		{
			T* flag = &elements[n];
			while (firstFree != flag)
			{
				alloc.destroy(--firstFree);
			}
		}
		else if (n > size())
		{
			check_n_cap(n);
			size_t numRest = n - size();
			while (numRest != 0)
			{
				alloc.construct(firstFree++,value);
				numRest--;
			}
		}
	}

	template <typename T>
	void Vec<T>::push_back(const T& _data)
	{
#ifndef CLEAR_CHEAT
		check();
		alloc.construct(firstFree++, _data);
#else   
		if( firstNotConstruct <= firstFree )
		{
			check();
			alloc.construct(firstFree++, _data);
			firstNotConstruct = firstFree;
		}
		else 
		{
			*firstFree = _data;
			firstFree++;
		}
#endif 
	}

	template <typename T>
	void Vec<T>::clear()
	{
		Free();
		elements = firstFree = cap = nullptr;
	}

	template <typename T>
	bool operator == (const Vec<T>& lhs, const Vec<T>& rhs)
	{
		if ( lhs.size() != rhs.size() )
			return false;
		T* it1 = lhs.begin();
		T* it2 = rhs.begin();
		T* itEnd1 = lhs.end();
		T* itEnd2 = rhs.end();
		while ( it1 != itEnd1 && it2 != itEnd2 && *it1 == *it2 ) 
		{
			it1++;
			it2++;
		}
		if ( it1 == itEnd1 && it2 == itEnd2 ) return true;
		return false;

		// for (; it1 != lhs.end() && it2 != rhs.end();  ++it1, ++it2)
		// {
		// 		if (*it1 != *it2) return false;
		// }
	}

	template <typename T> 
	bool operator != (const Vec<T>& lhs, const Vec<T>& rhs)
	{
		return !(lhs == rhs);
	}

	template <typename T>
	void Vec<T>::reallocate()
	{
		// std::cout << "在reallocate中，计算newCapacity之前，capacity()为" << capacity() << std::endl;
		size_t newCapacity = capacity() ? capacity() * 2 : 1;
		// std::cout << "在reallocate中，newCapacity = " << newCapacity << std::endl;
		T* newElements = alloc.allocate(newCapacity);
		T* newFirstFree = newElements;
		for ( T* it = elements; it != firstFree; ++it )
		{
			alloc.construct(newFirstFree++, std::move(*it));
		}

		Free();
		T* newCap = newElements + newCapacity;
		elements = newElements;
		firstFree = newFirstFree;
		cap = newCap;
		// std::cout << "在reallocate中，一次重新分配之后，容量是 " << capacity() << std::endl;
	}

	template <typename T>
	void Vec<T>::Free()
	{
		if (elements == nullptr) return;
		T* p = firstFree;
		while (p != elements)
			alloc.destroy(--p);
		alloc.deallocate(elements, capacity());
		return;
	}

	template <typename T>
	void Vec<T>::check()
	{
		if (firstFree == cap)
			reallocate();
	}

	template <typename T>
	void Vec<T>::check_n_cap(size_t n)
	{
		while (capacity() < n)
		{
			reallocate();
			// std::cout << "在check_n_cap中，重新分配了一次，目前容量是" << capacity() << std::endl;
			// getchar();
		}
			
	}

};

#endif // !__VEC__H