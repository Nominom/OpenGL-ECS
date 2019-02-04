#include "..\include\entitymanager.h"
#include "../include/world.h"
#include "../include/componentmanager.h"
#include "../include/eventmanager.h"


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
		arr.data = std::shared_ptr<Entity[]>(new Entity[number]);

		for (int i = 0; i < number; ++i) {
			arr.data[i].ID = EntityManager::NextID();
			_componentmanager->AddEntity(arr.data[i]);
			_eventmanager->QueueEvent(EntityCreatedEvent(arr.data[i]));
		}
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
		arr.data = std::shared_ptr<Entity[]>(new Entity[number]);

		for (int i = 0; i < number; ++i) {
			arr.data[i].ID = EntityManager::NextID();
			_componentmanager->AddEntity(arr.data[i], archetype);
			_eventmanager->QueueEvent(EntityCreatedEvent(arr.data[i]));
		}
	}

	return arr;
}

void EntityManager::Clear() {
	nextid = 1;
	freeIDs.clear();
}

void EntityManager::DestroyEntity(const Entity& entity) {
	_componentmanager->RemoveEntity(entity);
	freeIDs.push_back(entity.ID);
	_eventmanager->QueueEvent(EntityDestroyedEvent(entity));
}

void EntityManager::DestroyEntites(const EntityArray& entities) {

	for (const Entity& entity : entities) {
		_componentmanager->RemoveEntity(entity);
		freeIDs.push_back(entity.ID);
		_eventmanager->QueueEvent(EntityDestroyedEvent(entity));
	}
}

