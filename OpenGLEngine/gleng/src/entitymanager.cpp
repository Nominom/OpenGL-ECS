#include "..\include\entitymanager.h"
#include "../include/world.h"
#include "../include/componentmanager.h"
#include "../include/eventmanager.h"

uint32_t EntityManager::nextid = 1;

EntityManager::EntityManager(ComponentManager * cm, EventManager * em) {
	_componentmanager = cm;
	_eventmanager = em;
}

Entity EntityManager::CreateEntity() {
	Entity entity;
	entity.ID = EntityManager::NextID();

	_componentmanager->AddEntity(entity);
	_eventmanager->QueueEvent(EntityCreatedEvent(entity));

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
			_componentmanager->AddEntity(entities[i]);
			_eventmanager->QueueEvent(EntityCreatedEvent(entities[i]));

		}
		arr.data = std::shared_ptr<Entity[]> (new Entity[number]);
		std::memcpy(arr.data.get(), &entities[oldSize], number * sizeof(Entity));
	}
	
	return arr;
}

Entity EntityManager::CreateEntity(const EntityArchetype &archetype) {
	Entity entity;
	entity.ID = EntityManager::NextID();

	_componentmanager->AddEntity(entity, archetype);

	_eventmanager->QueueEvent(EntityCreatedEvent(entity));

	return entity;
}

EntityArray EntityManager::CreateEntitites(size_t number, const EntityArchetype &archetype) {
	EntityArray arr;
	arr.size = number;

	if (number > 0) {
		size_t oldSize = entities.size();
		size_t newSize = oldSize + number;
		entities.resize(newSize);
		for (int i = oldSize; i < newSize; i++) {
			entities[i].ID = EntityManager::NextID();
			_componentmanager->AddEntity(entities[i], archetype);
			_eventmanager->QueueEvent(EntityCreatedEvent(entities[i]));
		}
		arr.data = std::shared_ptr<Entity[]>(new Entity[number]);
		std::memcpy(arr.data.get(), &entities[oldSize], number * sizeof(Entity));
	}

	return arr;
}

void EntityManager::Clear() {
	nextid = 1;
	entities.clear();
}
