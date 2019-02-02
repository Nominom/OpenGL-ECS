#pragma once
#include "util.h"

#define CHECK_T_IS_COMPONENT static_assert(std::is_base_of<IComponent<T>, T>::value, "T is not of type Component");
#define CHECK_T_IS_SHARED_COMPONENT static_assert(std::is_base_of<ISharedComponent<T>, T>::value, "T is not of type SharedComponent");


template <class T>
struct IComponent {
	static const type_hash ComponentTypeID;
};

template <class T>
struct ISharedComponent{
	static const type_hash ComponentTypeID;
};

template <class T>
const type_hash IComponent<T>::ComponentTypeID = util::GetTypeHash<T>();


template <class T>
const type_hash ISharedComponent<T>::ComponentTypeID = util::GetTypeHash<T>();

class EntityArchetype;

class ComponentFilter {
	std::vector<type_hash> includeTypes;
	std::vector<type_hash> includeSharedTypes;
	std::vector<type_hash> excludeTypes;
	std::vector<type_hash> excludeSharedTypes;
public:
	/*
	template <class T>
	inline ComponentFilter& Include() {
		CHECK_T_IS_COMPONENT;
		includeTypes.push_back(IComponent<T>::ComponentTypeID);
		return *this;
	}*/

	template <class T, std::enable_if_t<std::is_base_of<IComponent<T>, T>::value, int> = 0>
	inline ComponentFilter& Include() {
		CHECK_T_IS_COMPONENT;
		includeTypes.push_back(IComponent<T>::ComponentTypeID);
		return *this;
	}

	template <class T, std::enable_if_t<std::is_base_of<ISharedComponent<T>, T>::value, int> = 0>
	inline ComponentFilter& Include() {
		CHECK_T_IS_SHARED_COMPONENT;
		includeSharedTypes.push_back(ISharedComponent<T>::ComponentTypeID);
		return *this;
	}

	template <class T, std::enable_if_t<std::is_base_of<IComponent<T>, T>::value, int> = 0>
	inline ComponentFilter& Exclude() {
		CHECK_T_IS_COMPONENT;
		excludeTypes.push_back(IComponent<T>::ComponentTypeID);
		return *this;
	}

	template <class T, std::enable_if_t<std::is_base_of<ISharedComponent<T>, T>::value, int> = 0>
	inline ComponentFilter& Exclude() {
		CHECK_T_IS_SHARED_COMPONENT;
		excludeSharedTypes.push_back(ISharedComponent<T>::ComponentTypeID);
		return *this;
	}

	bool Matches(const EntityArchetype& archetype) const;
};
/*
template <class T, std::enable_if_t<true, int>>
inline ComponentFilter& ComponentFilter::Include() {
	CHECK_T_IS_COMPONENT;
	includeTypes.push_back(IComponent<T>::ComponentTypeID);
	return *this;
}*/