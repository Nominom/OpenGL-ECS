#pragma once
#include <array>
#include <unordered_set>
#include <unordered_map>
#include "component.h"
#include <functional>

#ifndef ECS_NO_TSL
#include "../tsl/robin_map.h"
#endif

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

namespace util {
	namespace entityarchetype {
		template <typename... Components>
		struct EntityArchetypeCreator;

		template <typename... Shared>
		struct EntityArchetypeCreatorShared;
	}
}

class EntityArchetype {
#ifdef ECS_NO_TSL
	std::unordered_map<type_hash, size_t, util::typehasher> componentTypesMemory;
	std::unordered_map<type_hash, void*, util::typehasher> sharedComponents;
#else
	tsl::robin_map<type_hash, size_t, util::typehasher> componentTypesMemory;
	tsl::robin_map<type_hash, void*, util::typehasher> sharedComponents;
#endif // ECS_NO_TSL

	type_hash _archetypeHash = 0;

	inline void GenerateHash() {
		type_hash finalHash = 0;
		for (auto hash : componentTypesMemory) {
			finalHash ^= hash.first;
		}
		static std::hash<void*> hasher;
		for (auto pair : sharedComponents) {
			size_t valueHash = hasher(pair.second);
			finalHash ^= pair.first;
			finalHash ^= valueHash;
		}
		_archetypeHash = finalHash;
	}

public:
	inline EntityArchetype() {
		_archetypeHash = 0;
	}

	inline EntityArchetype(const ComponentType& component) {
		componentTypesMemory.emplace(component.type, component.memorySize);
		GenerateHash();
	}

	inline EntityArchetype(const EntityArchetype &other) {
#ifdef ECS_NO_TSL
		componentTypesMemory = std::unordered_map<type_hash, size_t, util::typehasher>(other.componentTypesMemory);
		sharedComponents = std::unordered_map<type_hash, void*, util::typehasher>(other.sharedComponents);
#else
		componentTypesMemory = tsl::robin_map<type_hash, size_t, util::typehasher>(other.componentTypesMemory);
		sharedComponents = tsl::robin_map<type_hash, void*, util::typehasher>(other.sharedComponents);
#endif // ECS_NO_TSL
		_archetypeHash = other._archetypeHash;
	}

	inline bool HasComponentType(type_hash componentType) const{
		auto ptr = componentTypesMemory.find(componentType);
		return ptr != componentTypesMemory.end();
	}

	inline bool HasSharedComponentType(type_hash sharedComponentType) const{
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

#ifdef ECS_NO_TSL
	inline const std::unordered_map<type_hash, size_t, util::typehasher> &GetComponentTypes() const {
		return componentTypesMemory;
}
#else
	inline const tsl::robin_map<type_hash, size_t, util::typehasher> &GetComponentTypes() const {
		return componentTypesMemory;
	}
#endif // ECS_NO_TSL

	inline type_hash ArchetypeHash() const{
		return _archetypeHash;
	}

	template <class ...Components, class ...SharedComponents>
	static EntityArchetype Create(SharedComponents*... shared) {
		EntityArchetype archetype = util::entityarchetype::EntityArchetypeCreator<Components...>::get();
		EntityArchetype withShared =
			util::entityarchetype::EntityArchetypeCreatorShared<SharedComponents...>::get(archetype, shared...);
		return withShared;
	}
	
};

namespace util{
	namespace entityarchetype{
		template <typename T, typename... Components>
		struct EntityArchetypeCreator<T, Components...> {

			static EntityArchetype get() {
				CHECK_T_IS_COMPONENT;
				EntityArchetype archetype = EntityArchetypeCreator<Components...>::get();
				return archetype.AddComponent(ComponentType::Get<T>());
			}
		};

		template <>
		struct EntityArchetypeCreator<> {
			static EntityArchetype get() {
				return EntityArchetype();
			}
		};

		template <typename T, typename... Shared>
		struct EntityArchetypeCreatorShared<T, Shared...> {

			static EntityArchetype get(const EntityArchetype& arc, T* shared, Shared*... rest) {
				CHECK_T_IS_SHARED_COMPONENT;
				EntityArchetype archetype = EntityArchetypeCreatorShared<Shared...>::get(arc, rest...);
				return archetype.AddSharedComponent(shared);
			}
		};

		template <>
		struct EntityArchetypeCreatorShared<> {
			static EntityArchetype get(const EntityArchetype& arc) {
				return arc;
			}
		};
	}
}

namespace std {

	template <>
	struct hash<EntityArchetype> {
		std::size_t operator()(const EntityArchetype& archetype) const {
			return archetype.ArchetypeHash();
		}
	};

}

