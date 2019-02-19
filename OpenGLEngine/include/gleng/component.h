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
	virtual ~ISharedComponent() = default;
};

template <class T>
const type_hash IComponent<T>::ComponentTypeID = util::GetTypeHash<T>();


template <class T>
const type_hash ISharedComponent<T>::ComponentTypeID = util::GetTypeHash<T>();


/*
template <class T, std::enable_if_t<true, int>>
inline ComponentFilter& ComponentFilter::Include() {
	CHECK_T_IS_COMPONENT;
	includeTypes.push_back(IComponent<T>::ComponentTypeID);
	return *this;
}*/