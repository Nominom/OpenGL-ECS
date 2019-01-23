#pragma once
#include "componentmanager.h"

template <typename T, typename Enable = void> 
struct ComponentDataIterator;

template <typename T>
struct ComponentDataIterator<T, typename std::enable_if<std::is_base_of<ISharedComponent<T>, T>::value>::type> {

	T* const component;

	inline ComponentDataIterator(ComponentMemoryBlock *block): component(block->type.GetSharedComponent<T>()){
		CHECK_T_IS_SHARED_COMPONENT;
	}

};

template <typename T> 
struct ComponentDataIterator<T, typename std::enable_if<std::is_base_of<IComponent<T>, T>::value>::type> {
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



/*
template <class T>
struct ComponentDataIterator {
	T* data;
	const size_t len;

	ComponentDataIterator(ComponentMemoryBlock *block) : len(block->size()) {
		CHECK_T_IS_COMPONENT;
		data = block->GetComponentArray<T>();
	}

	template<class Q = T>
	typename std::enable_if<std::is_base_of<IComponent<Q>, Q>::value, Q*>::type
	begin() {
		return data;
	}

	template<class Q = T>
	typename std::enable_if<std::is_base_of<IComponent<Q>, Q>::value, Q*>::type
	end() {
		return data + len;
	}

	template<class Q = T>
	typename std::enable_if<std::is_base_of<IComponent<Q>, Q>::value, Q&>::type
	operator [](size_t index) {
		assert(index < len);
		return data[index];
	}

	template<class Q = T>
	typename std::enable_if<std::is_base_of<ISharedComponent<Q>, Q>::value, Q&>::type
	GetSharedComponent() {
		static T t;
		return t;
	}
};

template <>
struct ComponentDataIterator<std::enable_if<std::is_base_of<IComponent<T>, T>::value, > {

};

*/
/*
template <class T>
struct ComponentDataIterator<IComponent<T>> {
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

template <class T>
struct ComponentDataIterator<ISharedComponent<T>> {

};*/




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

	virtual inline ComponentFilter GetFilter() {
		ComponentFilter filter;
		auto _ = { filter.Include<Components>()... };
		return filter;
	}
};

/*
template<typename T, typename... Ts>
constexpr bool contains() {
	return std::disjunction_v<std::is_same<T, Ts>...>;
}

static_assert(contains<T, Components...>());
return std::get<ComponentDataIterator<T>>(data);
*/