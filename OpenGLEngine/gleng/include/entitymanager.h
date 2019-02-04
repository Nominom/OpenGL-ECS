#pragma once
#include <vector>
#include "entity.h"
#include "entityarchetypes.h"

class ComponentManager;
class EventManager;


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
	EntityManager(ComponentManager*, EventManager*);
	Entity CreateEntity();
	EntityArray CreateEntitites(size_t number);
	EntityArray CreateEntitites(size_t number, const EntityArchetype&);
	Entity CreateEntity(const EntityArchetype&);
	void DestroyEntity(const Entity& entity);
	void DestroyEntites(const EntityArray &entities);
	void Clear();
};