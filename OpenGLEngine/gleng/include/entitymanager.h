#pragma once
#include <vector>
#include "entity.h"
#include "entityarchetypes.h"


class EntityManager {
	std::vector<Entity> entities;
	//std::vector<uint32_t> freeIDs;
	static uint32_t nextid;
	static uint32_t NextID() {
		return nextid++;
	}
public:
	Entity CreateEntity();
	EntityArray CreateEntitites(size_t number);
	Entity CreateEntity(const EntityArchetype&);
	EntityArray CreateEntitites(const EntityArchetype&, size_t number);
	void DestroyEntity(Entity entity);
	void DestroyEntites(EntityArray entitities);
	bool EntityExists(Entity entity) const;
	void Clear();
};