#pragma once
#include "componentmanager.h"



template <class T>
struct ComponentDataIterator {
	T* data;
	const size_t len;

	inline ComponentDataIterator(ComponentMemoryBlock *block) : len(block->size()) {
		CHECK_T_IS_COMPONENT;
		data = block->GetComponentArray<T>();
	}

	inline T* begin() {
		return data;
	}

	inline T* end() {
		return data + len;
	}

	inline T& operator [](size_t index) {
		assert(index < len);
		return data[index];
	}
};

struct EntityIterator {
	const Entity* data;
	const size_t len;

	inline EntityIterator(ComponentMemoryBlock *block) : len(block->size()) {
		data = block->GetEntityArray();
	}

	inline const Entity* begin() {
		return data;
	}

	inline const Entity* end() {
		return data + len;
	}

	inline const Entity& operator [](size_t index) {
		assert(index < len);
		return data[index];
	}
};

template <class ...Components>
class ComponentDataBlockArray {
private:
	size_t len;
	std::tuple<ComponentDataIterator<Components>...> data;
	EntityIterator entities;
public:
	
	inline ComponentDataBlockArray(ComponentMemoryBlock *block) : 
		data(ComponentDataIterator<Components>(block)...), entities(block){
		len = block->size();
	}

	template <class T>
	inline ComponentDataIterator<T> Get() const{
		return std::get<ComponentDataIterator<T>>(data);
	}

	inline size_t size() const{
		return len;
	}

	inline EntityIterator GetEntities() const {
		return entities;
	}
};

template <class ...Components>
class IComponentSystem {
public:
	bool running = true;
	virtual void DoWork(const ComponentDataBlockArray<Components...>&) = 0;
};

/*
template<typename T, typename... Ts>
constexpr bool contains() {
	return std::disjunction_v<std::is_same<T, Ts>...>;
}

static_assert(contains<T, Components...>());
return std::get<ComponentDataIterator<T>>(data);
*/