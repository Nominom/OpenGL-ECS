#pragma once
#include "events.h"

template <class T>
struct EventIterator {
	const T* const data;
	const size_t size;

	inline EventIterator(T* d, size_t size) : data(d), size(size){
		CHECK_T_IS_EVENT;
	}

	inline const T* begin() const{
		return data;
	}

	inline const T* end() const{
		return data + size;
	}

	inline const T& operator [](size_t index) const{
		assert(index < size);
		return data[index];
	}
};


template <class T>
class IEventListener {
public:
	virtual void ProcessEvents(const EventIterator<T>&) = 0;
};