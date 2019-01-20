#pragma once
#include "util.h"

#define CHECK_T_IS_COMPONENT static_assert(std::is_base_of<IComponent<T>, T>::value, "T is not of type Component");
#define CHECK_T_IS_SHARED_COMPONENT static_assert(std::is_base_of<ISharedComponent<T>, T>::value, "T is not of type SharedComponent");


template <class T>
class IComponent {
public:
	static const type_hash ComponentTypeID;
};

template <class T>
class ISharedComponent{
public:
	static const type_hash ComponentTypeID;
};

template <class T>
const type_hash IComponent<T>::ComponentTypeID = util::GetTypeHash<T>();


template <class T>
const type_hash ISharedComponent<T>::ComponentTypeID = util::GetTypeHash<T>();
