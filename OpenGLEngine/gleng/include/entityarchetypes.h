#pragma once
#include <array>
#include <unordered_set>
#include <unordered_map>
#include "component.h"

struct ComponentType {
private:
	ComponentType() = default;
public:
	type_hash type;
	size_t memorySize;
	template <class T>
	static ComponentType Get(){
		CHECK_T_IS_COMPONENT;

		static ComponentType ctype;
		ctype.type = IComponent<T>::ComponentTypeID;
		ctype.memorySize = sizeof(T);
		return ctype;
	}

	inline bool operator ==(const ComponentType& other) {
		return type == other.type;
	}

};

class EntityArchetype {
	std::unordered_map<type_hash, size_t> componentTypesMemory;
	std::unordered_map<type_hash, void*> sharedComponents;
	void GenerateHash();
	type_hash _archetypeHash;
public:
	inline EntityArchetype() {
		_archetypeHash = 0;
	}

	inline EntityArchetype(const ComponentType& component) {
		componentTypesMemory.emplace(component.type, component.memorySize);
		GenerateHash();
	}

	inline EntityArchetype(const EntityArchetype &other) {
		componentTypesMemory = std::unordered_map<type_hash, size_t>(other.componentTypesMemory);
		sharedComponents = std::unordered_map<type_hash, void*>(other.sharedComponents);
		_archetypeHash = other._archetypeHash;
	}

	inline bool HasComponentType(type_hash componentType) const{
		auto ptr = componentTypesMemory.find(componentType);
		return ptr != componentTypesMemory.end();
	}

	inline bool HasSharedComponentType(type_hash sharedComponentType) {
		auto ptr = sharedComponents.find(sharedComponentType);
		return ptr != sharedComponents.end();
	}

	inline EntityArchetype AddComponent(const ComponentType& component) const{
		EntityArchetype newArch(*this);
		newArch.componentTypesMemory.emplace(component.type, component.memorySize);
		newArch.GenerateHash();
		return newArch;
	}

	inline EntityArchetype RemoveComponent(const ComponentType& component) const {
		EntityArchetype newArch(*this);
		auto found = newArch.componentTypesMemory.find(component.type);

		if (found != newArch.componentTypesMemory.end()) {
			newArch.componentTypesMemory.erase(found);
		}

		newArch.GenerateHash();
		return newArch;
	}

	template <class T>
	inline EntityArchetype AddSharedComponent(T* component) const {
		CHECK_T_IS_SHARED_COMPONENT;
		EntityArchetype newArch(*this);
		newArch.sharedComponents.emplace(ISharedComponent<T>::ComponentTypeID, component);
		newArch.GenerateHash();
		return newArch;
	}

	inline EntityArchetype RemoveSharedComponent(type_hash sharedComponentType) const {
		EntityArchetype newArch(*this);
		auto found = newArch.sharedComponents.find(sharedComponentType);

		if (found != newArch.sharedComponents.end()) {
			newArch.sharedComponents.erase(found);
		}

		newArch.GenerateHash();
		return newArch;
	}
	template <class T>
	inline T* GetSharedComponent() const{
		CHECK_T_IS_SHARED_COMPONENT;
		auto found = sharedComponents.find(ISharedComponent<T>::ComponentTypeID);
		if (found != sharedComponents.end()) {
			return static_cast<T*>(found->second);
		} else {
			return nullptr;
		}
	}

	inline const std::unordered_map<type_hash, size_t> &GetComponentTypes() const {
		return componentTypesMemory;
	}

	inline type_hash ArchetypeHash() const{
		return _archetypeHash;
	}
};

namespace std {

	template <>
	struct hash<EntityArchetype> {
		std::size_t operator()(const EntityArchetype& archetype) const {
			return archetype.ArchetypeHash();
		}
	};

}
