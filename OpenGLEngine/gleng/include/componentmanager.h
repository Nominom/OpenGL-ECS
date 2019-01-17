#pragma once
#include "entity.h"
#include "component.h"
#include "entityarchetypes.h"
#include "memoryblocks.h"


struct ArchetypeBlockIndex {
	size_t archetypeIndex;
	size_t blockIndex;
	size_t elementIndex;
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
	std::unordered_map<type_hash, size_t> _archetypeHashIndices;

	//std::unordered_map<type_hash, ComponentMemoryBlock> _archetypeComponents;
	//std::unordered_map<type_hash, size_t> _archetypeHashIndices;
	//std::unordered_map<Entity, type_hash> _entityArchetypes;

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

public:

	inline ComponentManager() {
		static const EntityArchetype empty;
		CreateNewArchetypeBlock(empty);
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
		return GetMemoryBlock(newBlock)->GetComponentArray<T>()[newBlock.elementIndex];
	}

	template<class T>
	inline void RemoveComponent(const Entity& e) {
		CHECK_T_IS_COMPONENT;

		ArchetypeBlockIndex oldBlock = FindBlockIndexFor(e);

		ArchetypeBlockIndex newBlock;

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
};