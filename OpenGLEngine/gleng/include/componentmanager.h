#pragma once
#include "entity.h"
#include "component.h"
#include "entityarchetypes.h"
#include "memoryblocks.h"
#include "eventmanager.h"

#ifndef ECS_NO_TSL
#include "tsl/robin_map.h"
#endif


struct ArchetypeBlockIndex {
	size_t archetypeIndex;
	size_t blockIndex;
	size_t elementIndex;

	static ArchetypeBlockIndex Invalid() {
		return ArchetypeBlockIndex();
	}
};

class EntityArchetypeBlock {
public:
	EntityArchetype archetype;
	std::vector<ComponentMemoryBlock*> archetypeBlocks;

	inline EntityArchetypeBlock(EntityArchetype type) {
		archetype = type;
	}

	inline size_t CreateNewBlockIndex() {
		static MemoryBlockAllocator &allocator = MemoryBlockAllocator::instance();

		ComponentMemoryBlock *newBlock = allocator.Allocate();

		newBlock->Initialize(archetype);

		archetypeBlocks.push_back(newBlock);

		return archetypeBlocks.size() - 1;
	}

	inline size_t GetOrCreateFreeBlockIndex() {
		for (size_t i = 0; i < archetypeBlocks.size(); i++) {
			ComponentMemoryBlock* block = archetypeBlocks[i];
			if (block->size() < block->maxSize()) {
				return i;
			}
		}
		//couldn't find free block
		return CreateNewBlockIndex();
	}
};


class ComponentManager {
	std::vector<EntityArchetypeBlock> _archetypes;
	std::vector<ArchetypeBlockIndex> _entityMap;
#ifdef ECS_NO_TSL
	std::unordered_map<type_hash, size_t, util::typehasher> _archetypeHashIndices;
#else
	tsl::robin_map<type_hash, size_t, util::typehasher> _archetypeHashIndices;
#endif // ECS_NO_TSL

	EventManager *_eventmanager;


	inline ArchetypeBlockIndex GetFreeBlockOf(const EntityArchetype& archetype) {
		ArchetypeBlockIndex newArchIndex;

		auto found = _archetypeHashIndices.find(archetype.ArchetypeHash());
		if (found == _archetypeHashIndices.end()) {
			size_t idx = CreateNewArchetypeBlock(archetype);
			newArchIndex.archetypeIndex = idx;
		} else {
			newArchIndex.archetypeIndex = found->second;
		}

		newArchIndex.blockIndex = _archetypes[newArchIndex.archetypeIndex].GetOrCreateFreeBlockIndex();

		return newArchIndex;
	}

	inline EntityArchetypeBlock& FindArchetypeFor(const Entity &e) {
		const ArchetypeBlockIndex& idx = _entityMap[e.ID];
		return _archetypes[idx.archetypeIndex];
	}

	inline ComponentMemoryBlock* FindComponentBlockFor(const Entity& e) {
		const ArchetypeBlockIndex& idx = _entityMap[e.ID];

		return _archetypes[idx.archetypeIndex].archetypeBlocks[idx.blockIndex];
	}

	inline ArchetypeBlockIndex FindBlockIndexFor(const Entity& e) {
		return _entityMap[e.ID];
	}

	inline size_t CreateNewArchetypeBlock(const EntityArchetype& archetype) {
		_archetypes.push_back(EntityArchetypeBlock(archetype));
		size_t idx = _archetypes.size() - 1;
		_archetypeHashIndices.emplace(archetype.ArchetypeHash(), idx);
		return idx;
	}

	inline size_t ArchetypeAddComponent(const EntityArchetype& archetype, const ComponentType& component) {
		type_hash newHash = archetype.ArchetypeHash() ^ component.type;
		
		auto found = _archetypeHashIndices.find(newHash);
		if (found == _archetypeHashIndices.end()) {
			EntityArchetype newArchetype = archetype.AddComponent(component);

			size_t idx = CreateNewArchetypeBlock(newArchetype);
			return idx;
		} else {
			return found->second;
		}
	}

	inline size_t ArchetypeRemoveComponent(const EntityArchetype& archetype, const ComponentType& component) {
		type_hash newHash = archetype.ArchetypeHash() ^ component.type;

		auto found = _archetypeHashIndices.find(newHash);
		if (found == _archetypeHashIndices.end()) {
			EntityArchetype newArchetype = archetype.RemoveComponent(component);

			size_t idx = CreateNewArchetypeBlock(newArchetype);
			return idx;
		} else {
			return found->second;
		}
	}

	inline size_t FindOrCreateArchetypeBlock(const EntityArchetype& archetype) {
		type_hash newHash = archetype.ArchetypeHash();

		auto found = _archetypeHashIndices.find(newHash);
		if (found == _archetypeHashIndices.end()) {
			size_t idx = CreateNewArchetypeBlock(archetype);
			return idx;
		} else {
			return found->second;
		}
	}

	inline ComponentMemoryBlock* GetMemoryBlock(const ArchetypeBlockIndex &idx) {
		return _archetypes[idx.archetypeIndex].archetypeBlocks[idx.blockIndex];
	}

	inline const EntityArchetype& GetArchetype(const ArchetypeBlockIndex &idx) {
		return _archetypes[idx.archetypeIndex].archetype;
	}

	template<class T>
	inline size_t ArchetypeAddSharedComponent(const EntityArchetype& archetype, T* component) {
		CHECK_T_IS_SHARED_COMPONENT;

		static std::hash<void*> hasher;
		type_hash newHash = archetype.ArchetypeHash() ^ ISharedComponent<T>::ComponentTypeID ^ hasher(component);

		auto found = _archetypeHashIndices.find(newHash);
		if (found == _archetypeHashIndices.end()) {
			EntityArchetype newArchetype = archetype.AddSharedComponent(component);

			size_t idx = CreateNewArchetypeBlock(newArchetype);
			return idx;
		} else {
			return found->second;
		}
	}

	template<class T>
	inline size_t ArchetypeRemoveSharedComponent(const EntityArchetype& archetype) {
		CHECK_T_IS_SHARED_COMPONENT;
		EntityArchetype newArchetype = archetype.RemoveSharedComponent(ISharedComponent<T>::ComponentTypeID);
		type_hash newHash = newArchetype.ArchetypeHash();

		auto found = _archetypeHashIndices.find(newHash);
		if (found == _archetypeHashIndices.end()) {
			size_t idx = CreateNewArchetypeBlock(newArchetype);
			return idx;
		} else {
			return found->second;
		}
	}

public:

	inline ComponentManager(EventManager* em) {
		_eventmanager = em;
	}

	inline void AddEntity(const Entity& e) {
		static const EntityArchetype empty;

		if (_entityMap.size() <= e.ID) {
			_entityMap.resize(e.ID + 1);
		}
		ArchetypeBlockIndex idx = GetFreeBlockOf(empty);
		idx.elementIndex = _archetypes[idx.archetypeIndex].archetypeBlocks[idx.blockIndex]->AddEntity(e);
		_entityMap[e.ID] = idx;
	}

	inline void AddEntity(const Entity& e, const EntityArchetype& archetype) {
		if (_entityMap.size() <= e.ID) {
			_entityMap.resize(e.ID + 1);
		}
		ArchetypeBlockIndex idx = GetFreeBlockOf(archetype);
		idx.elementIndex = _archetypes[idx.archetypeIndex].archetypeBlocks[idx.blockIndex]->AddEntity(e);
		_entityMap[e.ID] = idx;

#ifndef ECS_NO_COMPONENT_EVENTS
		for (std::pair<type_hash, size_t> component : _archetypes[idx.archetypeIndex].archetype.GetComponentTypes()) {
			ComponentAddedEvent ev;
			ev.entity = e;
			ev.componentType = component.first;
			_eventmanager->QueueEvent(ev);
		}
#endif //ECS_NO_COMPONENT_EVENTS
	}

	inline void RemoveEntity(const Entity& e) {
		ArchetypeBlockIndex idx = FindBlockIndexFor(e);

#ifndef ECS_NO_COMPONENT_EVENTS
		for (std::pair<type_hash, size_t> component : _archetypes[idx.archetypeIndex].archetype.GetComponentTypes()) {
			ComponentRemovedEvent ev;
			ev.entity = e;
			ev.componentType = component.first;
			_eventmanager->QueueEvent(ev);
		}
#endif //ECS_NO_COMPONENT_EVENTS

		Entity removedEntity = _archetypes[idx.archetypeIndex].archetypeBlocks[idx.blockIndex]->RemoveEntityMoveLast(idx.elementIndex);
		if (removedEntity.ID != ENTITY_NULL_ID) {
			_entityMap[removedEntity.ID].elementIndex = idx.elementIndex;
		}

		_entityMap[e.ID] = ArchetypeBlockIndex::Invalid();
	}

	template<class T>
	inline void AddComponentCopy(const Entity& e, const T& original) {
		CHECK_T_IS_COMPONENT;
		AddComponent<T>(e) = original;
	}

	template<class T>
	inline T& AddComponent(const Entity& e) {
		CHECK_T_IS_COMPONENT;

		ArchetypeBlockIndex oldBlock = FindBlockIndexFor(e);

		ArchetypeBlockIndex newBlock;

		assert(!HasComponent<T>(e));

		newBlock.archetypeIndex = ArchetypeAddComponent(
			GetArchetype(oldBlock), 
			ComponentType::Get<T>());

		newBlock.blockIndex = _archetypes[newBlock.archetypeIndex].GetOrCreateFreeBlockIndex();
		
		auto ob = GetMemoryBlock(oldBlock);
		auto nb = GetMemoryBlock(newBlock);
		newBlock.elementIndex = ob->CopyEntityTo(oldBlock.elementIndex, e, nb);

		Entity removedEntity = ob->RemoveEntityMoveLast(oldBlock.elementIndex);
		if (removedEntity.ID != ENTITY_NULL_ID) {
			_entityMap[removedEntity.ID].elementIndex = oldBlock.elementIndex;
		}

		_entityMap[e.ID] = newBlock;


#ifndef ECS_NO_COMPONENT_EVENTS
		ComponentAddedEvent ev;
		ev.entity = e;
		ev.componentType = IComponent<T>::ComponentTypeID;
		_eventmanager->QueueEvent(ev);
#endif //ECS_NO_COMPONENT_EVENTS


		return GetMemoryBlock(newBlock)->GetComponentArray<T>()[newBlock.elementIndex];
	}

	template<class T>
	inline void RemoveComponent(const Entity& e) {
		CHECK_T_IS_COMPONENT;

		ArchetypeBlockIndex oldBlock = FindBlockIndexFor(e);

		ArchetypeBlockIndex newBlock;

		assert(HasComponent<T>(e));


		newBlock.archetypeIndex = ArchetypeRemoveComponent(
			GetArchetype(oldBlock),
			ComponentType::Get<T>());

		newBlock.blockIndex = _archetypes[newBlock.archetypeIndex].GetOrCreateFreeBlockIndex();
		auto ob = GetMemoryBlock(oldBlock);
		auto nb = GetMemoryBlock(newBlock);
		newBlock.elementIndex = ob->CopyEntityTo(oldBlock.elementIndex, e, nb);

		Entity removedEntity = ob->RemoveEntityMoveLast(oldBlock.elementIndex);
		if (removedEntity.ID != ENTITY_NULL_ID) {
			_entityMap[removedEntity.ID].elementIndex = oldBlock.elementIndex;
		}
		_entityMap[e.ID] = newBlock;

#ifndef ECS_NO_COMPONENT_EVENTS
		ComponentRemovedEvent ev;
		ev.entity = e;
		ev.componentType = IComponent<T>::ComponentTypeID;
		_eventmanager->QueueEvent(ev);
#endif //ECS_NO_COMPONENT_EVENTS
	}

	inline void MoveToArchetype(const Entity &e, const EntityArchetype& archetype) {
		ArchetypeBlockIndex oldBlock = FindBlockIndexFor(e);

		ArchetypeBlockIndex newBlock;

		newBlock.archetypeIndex = FindOrCreateArchetypeBlock(archetype);

		newBlock.blockIndex = _archetypes[newBlock.archetypeIndex].GetOrCreateFreeBlockIndex();
		auto ob = GetMemoryBlock(oldBlock);
		auto nb = GetMemoryBlock(newBlock);

		newBlock.elementIndex = ob->CopyEntityTo(oldBlock.elementIndex, e, nb);
		Entity removedEntity = ob->RemoveEntityMoveLast(oldBlock.elementIndex);
		if (removedEntity.ID != ENTITY_NULL_ID) {
			_entityMap[removedEntity.ID].elementIndex = oldBlock.elementIndex;
		}
		_entityMap[e.ID] = newBlock;


#ifndef ECS_NO_COMPONENT_EVENTS

		auto &oldComponents = _archetypes[oldBlock.archetypeIndex].archetype.GetComponentTypes();
		auto &newComponents = _archetypes[newBlock.archetypeIndex].archetype.GetComponentTypes();

		for (auto oldC : oldComponents) {
			if (newComponents.find(oldC.first) == newComponents.end()) {
				ComponentRemovedEvent ev;
				ev.entity = e;
				ev.componentType = oldC.first;
				_eventmanager->QueueEvent(ev);
			}
		}

		for (auto newC : newComponents) {
			if (oldComponents.find(newC.first) == oldComponents.end()) {
				ComponentAddedEvent ev;
				ev.entity = e;
				ev.componentType = newC.first;
				_eventmanager->QueueEvent(ev);
			}
		}
#endif //ECS_NO_COMPONENT_EVENTS
	}

	template<class T>
	inline T& GetComponent(const Entity& e) {
		CHECK_T_IS_COMPONENT;

		ArchetypeBlockIndex index = FindBlockIndexFor(e);

		return _archetypes[index.archetypeIndex]
			.archetypeBlocks[index.blockIndex]
			->GetComponent<T>(index.elementIndex);
	}

	template<class T>
	inline bool HasComponent(const Entity & e) {
		CHECK_T_IS_COMPONENT;
		if (e.ID == ENTITY_NULL_ID) return false;
		
		return FindArchetypeFor(e).archetype.HasComponentType(IComponent<T>::ComponentTypeID);
	}


	//shared components
	template<class T>
	inline T* CreateSharedComponent() {
		CHECK_T_IS_SHARED_COMPONENT;
		static SharedComponentAllocator &allocator = SharedComponentAllocator::instance();
		return allocator.Allocate<T>();
	}

	template<class T>
	inline T* DestroySharedComponent() {
		CHECK_T_IS_SHARED_COMPONENT;
		static SharedComponentAllocator &allocator = SharedComponentAllocator::instance();
		return allocator.Allocate<T>();
	}

	template<class T>
	inline void AddSharedComponent(const Entity &e, T* component) {
		CHECK_T_IS_SHARED_COMPONENT;

		ArchetypeBlockIndex oldBlock = FindBlockIndexFor(e);

		ArchetypeBlockIndex newBlock;

		assert(!HasSharedComponent<T>(e));

		newBlock.archetypeIndex = ArchetypeAddSharedComponent(GetArchetype(oldBlock), component);

		newBlock.blockIndex = _archetypes[newBlock.archetypeIndex].GetOrCreateFreeBlockIndex();

		auto ob = GetMemoryBlock(oldBlock);
		auto nb = GetMemoryBlock(newBlock);
		newBlock.elementIndex = ob->CopyEntityTo(oldBlock.elementIndex, e, nb);

		Entity removedEntity = ob->RemoveEntityMoveLast(oldBlock.elementIndex);
		if (removedEntity.ID != ENTITY_NULL_ID) {
			_entityMap[removedEntity.ID].elementIndex = oldBlock.elementIndex;
		}

		_entityMap[e.ID] = newBlock;
	}

	template<class T>
	inline void RemoveSharedComponent(const Entity &e) {
		CHECK_T_IS_SHARED_COMPONENT;

		ArchetypeBlockIndex oldBlock = FindBlockIndexFor(e);

		ArchetypeBlockIndex newBlock;

		assert(HasSharedComponent<T>(e));

		newBlock.archetypeIndex = ArchetypeRemoveSharedComponent<T>(GetArchetype(oldBlock));

		newBlock.blockIndex = _archetypes[newBlock.archetypeIndex].GetOrCreateFreeBlockIndex();

		auto ob = GetMemoryBlock(oldBlock);
		auto nb = GetMemoryBlock(newBlock);
		newBlock.elementIndex = ob->CopyEntityTo(oldBlock.elementIndex, e, nb);

		Entity removedEntity = ob->RemoveEntityMoveLast(oldBlock.elementIndex);
		if (removedEntity.ID != ENTITY_NULL_ID) {
			_entityMap[removedEntity.ID].elementIndex = oldBlock.elementIndex;
		}

		_entityMap[e.ID] = newBlock;
	}

	template<class T>
	inline T* GetSharedComponent(const Entity &e) {
		CHECK_T_IS_SHARED_COMPONENT;
		T* ptr = FindArchetypeFor(e).archetype.GetSharedComponent<T>();
		assert(ptr != nullptr);
		return ptr;
	}

	template<class T>
	inline bool HasSharedComponent(const Entity &e) {
		CHECK_T_IS_SHARED_COMPONENT;
		return FindArchetypeFor(e).archetype.HasSharedComponentType(ISharedComponent<T>::ComponentTypeID);
	}

	inline std::vector<ComponentMemoryBlock*> GetMemoryBlocks(const ComponentFilter &filter) {
		std::vector<ComponentMemoryBlock*> result;
		for (const EntityArchetypeBlock &atype : _archetypes) {
			if (filter.Matches(atype.archetype)) {
				for (ComponentMemoryBlock *block : atype.archetypeBlocks) {
					result.push_back(block);
				}
			}
		}
		return result;
	}

	inline void Clear() {
		_archetypes.clear();
		_entityMap.clear();
		_archetypeHashIndices.clear();
		MemoryBlockAllocator::instance().Clear();
		SharedComponentAllocator::instance().Clear();
	}
};