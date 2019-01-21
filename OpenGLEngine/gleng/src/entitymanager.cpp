#include "..\include\entitymanager.h"
#include "../include/world.h"

uint32_t EntityManager::nextid = 1;

Entity EntityManager::CreateEntity() {
	Entity entity;
	entity.ID = EntityManager::NextID();

	World::GetComponentManager()->AddEntity(entity);
	return entity;
}

EntityArray EntityManager::CreateEntitites(size_t number) {
	EntityArray arr;
	arr.size = number;

	if (number > 0) {
		size_t oldSize = entities.size();
		size_t newSize = oldSize + number;
		entities.resize(newSize);
		for (int i = oldSize; i < newSize; i++) {
			entities[i].ID = EntityManager::NextID();
			World::GetComponentManager()->AddEntity(entities[i]);
		}
		arr.data = std::shared_ptr<Entity[]> (new Entity[number]);
		std::memcpy(arr.data.get(), &entities[oldSize], number * sizeof(Entity));
	}
	
	return arr;
}

void EntityManager::Clear() {
	nextid = 1;
	entities.clear();
}
