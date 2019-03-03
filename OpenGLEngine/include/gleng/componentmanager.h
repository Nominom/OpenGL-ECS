#pragma once
#include "entity.h"
#include "component.h"
#include "entityarchetypes.h"
#include "memoryblocks.h"
#include "eventmanager.h"
#include "componenteventspawner.h"
#include "componentquery.h"

#ifndef ECS_NO_TSL
#include "../tsl/robin_map.h"
#endif

namespace gleng {


	//TODO: invalid block index
	struct ArchetypeBlockIndex {
		bool valid = false;
		size_t archetypeIndex = 0;
		size_t blockIndex = 0;
		size_t elementIndex = 0;

		static ArchetypeBlockIndex Invalid() {
			ArchetypeBlockIndex index;
			index.valid = false;
			return index;
		}
	};

	class EntityArchetypeBlock {
	public:
		EntityArchetype archetype;
		std::vector<ComponentMemoryBlock*> archetypeBlocks;
		int lastUsedIdx = -1;

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
			if (lastUsedIdx != -1 && archetypeBlocks[lastUsedIdx]->HasRoom()) {
				return lastUsedIdx;
			}
			for (size_t i = 0; i < archetypeBlocks.size(); i++) {
				ComponentMemoryBlock* block = archetypeBlocks[i];
				if (block->HasRoom()) {
					lastUsedIdx = i;
					return i;
				}
			}
			//couldn't find free block
			lastUsedIdx = CreateNewBlockIndex();
			return lastUsedIdx;
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
			newArchIndex.valid = true;

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
			assert(idx.valid);
			return _archetypes[idx.archetypeIndex];
		}

		inline ComponentMemoryBlock* FindComponentBlockFor(const Entity& e) {
			const ArchetypeBlockIndex& idx = _entityMap[e.ID];
			assert(idx.valid);
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
				ComponentEventSpawner::instance().ComponentAdded(component.first, e, _eventmanager);
			}

			for (std::pair<type_hash, void*> sharedComponent : _archetypes[idx.archetypeIndex].archetype.GetSharedComponents()) {
				ComponentEventSpawner::instance().SharedComponentAdded(sharedComponent.first, e, sharedComponent.second, _eventmanager);
			}
#endif //ECS_NO_COMPONENT_EVENTS
		}

		inline void RemoveEntity(const Entity& e) {
			ArchetypeBlockIndex idx = FindBlockIndexFor(e);

#ifndef ECS_NO_COMPONENT_EVENTS
			for (std::pair<type_hash, size_t> component : _archetypes[idx.archetypeIndex].archetype.GetComponentTypes()) {
				ComponentEventSpawner::instance().ComponentRemoved(component.first, e, _eventmanager);
			}

			for (std::pair<type_hash, void*> sharedComponent : _archetypes[idx.archetypeIndex].archetype.GetSharedComponents()) {
				ComponentEventSpawner::instance().SharedComponentRemoved(sharedComponent.first, e, sharedComponent.second, _eventmanager);
			}
#endif //ECS_NO_COMPONENT_EVENTS

			Entity removedEntity = _archetypes[idx.archetypeIndex].archetypeBlocks[idx.blockIndex]->RemoveEntityMoveLast(idx.elementIndex);
			if (removedEntity.ID != ENTITY_NULL_ID) {
				_entityMap[removedEntity.ID].elementIndex = idx.elementIndex;
			}

			_entityMap[e.ID] = ArchetypeBlockIndex::Invalid();
		}

		inline bool IsEntityValid(const Entity& e) {
			if (e.ID < _entityMap.size()) {
				ArchetypeBlockIndex idx = FindBlockIndexFor(e);
				return idx.valid;
			} else {
				return false;
			}
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

			assert(oldBlock.valid);
			assert(!HasComponent<T>(e));

			newBlock.valid = true;
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
			ComponentEventSpawner::instance().ComponentAdded<T>(e, _eventmanager);
#endif //ECS_NO_COMPONENT_EVENTS

			return GetMemoryBlock(newBlock)->GetComponentArray<T>()[newBlock.elementIndex];
		}

		template<class T>
		inline void RemoveComponent(const Entity& e) {
			CHECK_T_IS_COMPONENT;

			ArchetypeBlockIndex oldBlock = FindBlockIndexFor(e);

			ArchetypeBlockIndex newBlock;

			assert(oldBlock.valid);
			assert(HasComponent<T>(e));


			newBlock.valid = true;
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
			ComponentEventSpawner::instance().ComponentRemoved<T>(e, _eventmanager);
#endif //ECS_NO_COMPONENT_EVENTS
		}

		inline void MoveToArchetype(const Entity &e, const EntityArchetype& archetype) {
			ArchetypeBlockIndex oldBlock = FindBlockIndexFor(e);

			ArchetypeBlockIndex newBlock;

			assert(oldBlock.valid);

			newBlock.valid = true;
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
					ComponentEventSpawner::instance().ComponentRemoved(oldC.first, e, _eventmanager);
				}
			}

			for (auto newC : newComponents) {
				if (oldComponents.find(newC.first) == oldComponents.end()) {
					ComponentEventSpawner::instance().ComponentAdded(newC.first, e, _eventmanager);
				}
			}


			auto &oldSharedComponents = _archetypes[oldBlock.archetypeIndex].archetype.GetSharedComponents();
			auto &newSharedComponents = _archetypes[newBlock.archetypeIndex].archetype.GetSharedComponents();
			for (auto oldC : oldSharedComponents) {
				if (newSharedComponents.find(oldC.first) == newSharedComponents.end()) {
					ComponentEventSpawner::instance().SharedComponentRemoved(oldC.first, e, oldC.second, _eventmanager);
				}
			}

			for (auto newC : newSharedComponents) {
				if (oldSharedComponents.find(newC.first) == oldSharedComponents.end()) {
					ComponentEventSpawner::instance().SharedComponentAdded(newC.first, e, newC.second, _eventmanager);
				}
			}
#endif //ECS_NO_COMPONENT_EVENTS
		}

		template<class T>
		inline T& GetComponent(const Entity& e) {
			CHECK_T_IS_COMPONENT;

			ArchetypeBlockIndex index = FindBlockIndexFor(e);

			assert(index.valid);

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
		inline void DestroySharedComponent(T* component) {
			CHECK_T_IS_SHARED_COMPONENT;
			static SharedComponentAllocator &allocator = SharedComponentAllocator::instance();
			return allocator.Deallocate<T>(component);
		}

		template<class T>
		inline void AddSharedComponent(const Entity &e, T* component) {
			CHECK_T_IS_SHARED_COMPONENT;

			ArchetypeBlockIndex oldBlock = FindBlockIndexFor(e);

			ArchetypeBlockIndex newBlock;

			assert(oldBlock.valid);
			assert(!HasSharedComponent<T>(e));

			newBlock.valid = true;

			newBlock.archetypeIndex = ArchetypeAddSharedComponent(GetArchetype(oldBlock), component);

			newBlock.blockIndex = _archetypes[newBlock.archetypeIndex].GetOrCreateFreeBlockIndex();

			auto ob = GetMemoryBlock(oldBlock);
			auto nb = GetMemoryBlock(newBlock);
			newBlock.elementIndex = ob->CopyEntityTo(oldBlock.elementIndex, e, nb);

			Entity removedEntity = ob->RemoveEntityMoveLast(oldBlock.elementIndex);
			if (removedEntity.ID != ENTITY_NULL_ID) {
				_entityMap[removedEntity.ID].elementIndex = oldBlock.elementIndex;
			}

#ifndef ECS_NO_COMPONENT_EVENTS
			ComponentEventSpawner::instance().SharedComponentAdded<T>(e, component, _eventmanager);
#endif //ECS_NO_COMPONENT_EVENTS

			_entityMap[e.ID] = newBlock;
		}

		template<class T>
		inline void RemoveSharedComponent(const Entity &e) {
			CHECK_T_IS_SHARED_COMPONENT;

			ArchetypeBlockIndex oldBlock = FindBlockIndexFor(e);

			ArchetypeBlockIndex newBlock;

			assert(oldBlock.valid);
			assert(HasSharedComponent<T>(e));

			newBlock.valid = true;

			newBlock.archetypeIndex = ArchetypeRemoveSharedComponent<T>(GetArchetype(oldBlock));

			newBlock.blockIndex = _archetypes[newBlock.archetypeIndex].GetOrCreateFreeBlockIndex();

			auto ob = GetMemoryBlock(oldBlock);
			auto nb = GetMemoryBlock(newBlock);
			newBlock.elementIndex = ob->CopyEntityTo(oldBlock.elementIndex, e, nb);

			Entity removedEntity = ob->RemoveEntityMoveLast(oldBlock.elementIndex);
			if (removedEntity.ID != ENTITY_NULL_ID) {
				_entityMap[removedEntity.ID].elementIndex = oldBlock.elementIndex;
			}

#ifndef ECS_NO_COMPONENT_EVENTS
			ComponentEventSpawner::instance().SharedComponentRemoved<T>(e, GetArchetype(oldBlock).GetSharedComponent<T>(), _eventmanager);
#endif //ECS_NO_COMPONENT_EVENTS

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

		inline std::vector<ComponentMemoryBlock*> GetMemoryBlocks(const ComponentQuery &query) {
			std::vector<ComponentMemoryBlock*> result;
			for (const EntityArchetypeBlock &atype : _archetypes) {
				if (query.Matches(atype.archetype)) {
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

}