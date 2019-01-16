#pragma once
#include <vector>
#include "entity.h"

class EntityManager {
	std::vector<Entity> entities;

	static uint32_t NextID() {
		static uint32_t next = 1;
		return next++;
	}
public:
	Entity CreateEntity();
	EntityArray CreateEntitites(size_t number);
	void DestroyEntity(Entity entity);
	void DestroyEntites(EntityArray entitities);
	bool EntityExists(Entity entity) const;


};