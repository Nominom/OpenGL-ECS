#pragma once
#include <vector>
#include "entity.h"
#include "entityarchetypes.h"

class ComponentManager;
class EventManager;


class EntityManager {
	std::vector<Entity> entities;
	//std::vector<uint32_t> freeIDs;
	static uint32_t nextid;
	static uint32_t NextID() {
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
	void DestroyEntity(Entity entity);
	void DestroyEntites(EntityArray entitities);
	bool EntityExists(Entity entity) const;
	void Clear();
};