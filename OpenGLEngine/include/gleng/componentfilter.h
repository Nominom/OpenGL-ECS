#pragma once
#include <vector>
#include "component.h"
#include "entityarchetypes.h"


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

	inline bool Matches(const EntityArchetype& archetype) const {
		for (type_hash t : includeTypes) {
			if (!archetype.HasComponentType(t)) {
				return false;
			}
		}

		for (type_hash t : includeSharedTypes) {
			if (!archetype.HasSharedComponentType(t)) {
				return false;
			}
		}

		for (type_hash t : excludeTypes) {
			if (archetype.HasComponentType(t)) {
				return false;
			}
		}

		for (type_hash t : excludeSharedTypes) {
			if (archetype.HasSharedComponentType(t)) {
				return false;
			}
		}

		return true;
	}
};