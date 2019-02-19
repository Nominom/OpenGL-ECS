#pragma once
#include <vector>
#include "entity.h"
#include "entityarchetypes.h"
#include "componentmanager.h"
#include "eventmanager.h"

namespace gleng {

	class EntityManager {
		std::vector<uint32_t> freeIDs;
		uint32_t nextid = 1;
		inline uint32_t NextID() {
			if (freeIDs.size() > 0) {
				uint32_t lastIdx = freeIDs[freeIDs.size() - 1];
				freeIDs.pop_back();
				return lastIdx;
			}
			return nextid++;
		}

		EventManager *_eventmanager;
		ComponentManager *_componentmanager;
	public:

		inline EntityManager(ComponentManager * cm, EventManager * em) {
			_componentmanager = cm;
			_eventmanager = em;
		}

		inline Entity CreateEntity() {
			Entity entity;
			entity.ID = EntityManager::NextID();

			_componentmanager->AddEntity(entity);
			_eventmanager->QueueEvent(EntityCreatedEvent(entity));

			return entity;
		}

		inline EntityArray CreateEntities(size_t number) {
			EntityArray arr;
			arr.size = number;

			if (number > 0) {
				arr.data = std::shared_ptr<Entity[]>(new Entity[number]);

				for (int i = 0; i < number; ++i) {
					arr.data[i].ID = EntityManager::NextID();
					_componentmanager->AddEntity(arr.data[i]);
					_eventmanager->QueueEvent(EntityCreatedEvent(arr.data[i]));
				}
			}

			return arr;
		}

		inline EntityArray CreateEntities(size_t number, const EntityArchetype &archetype) {
			EntityArray arr;
			arr.size = number;

			if (number > 0) {
				arr.data = std::shared_ptr<Entity[]>(new Entity[number]);

				for (int i = 0; i < number; ++i) {
					arr.data[i].ID = EntityManager::NextID();
					_componentmanager->AddEntity(arr.data[i], archetype);
					_eventmanager->QueueEvent(EntityCreatedEvent(arr.data[i]));
				}
			}

			return arr;
		}

		inline Entity CreateEntity(const EntityArchetype &archetype) {
			Entity entity;
			entity.ID = EntityManager::NextID();

			_componentmanager->AddEntity(entity, archetype);

			_eventmanager->QueueEvent(EntityCreatedEvent(entity));

			return entity;
		}

		inline void DestroyEntity(const Entity& entity) {
			//TODO: check that is invalid block index in componentmanager
			//assert(std::find(freeIDs.begin(), freeIDs.end(), entity.ID) == freeIDs.end());
			assert(_componentmanager->IsEntityValid(entity));
			assert(entity.ID != ENTITY_NULL_ID);

			_componentmanager->RemoveEntity(entity);
			freeIDs.push_back(entity.ID);
			_eventmanager->QueueEvent(EntityDestroyedEvent(entity));
		}

		inline void DestroyEntities(const EntityArray& entities) {
			//TODO: check that is invalid block index in componentmanager
			for (const Entity& entity : entities) {
				assert(_componentmanager->IsEntityValid(entity));
				//assert(std::find(freeIDs.begin(), freeIDs.end(), entity.ID) == freeIDs.end());
				assert(entity.ID != ENTITY_NULL_ID);

				_componentmanager->RemoveEntity(entity);
				freeIDs.push_back(entity.ID);
				_eventmanager->QueueEvent(EntityDestroyedEvent(entity));
			}
		}

		inline bool IsAlive(const Entity & entity) {
			return _componentmanager->IsEntityValid(entity);
		}

		inline void Clear() {
			nextid = 1;
			freeIDs.clear();
		}
	};

}