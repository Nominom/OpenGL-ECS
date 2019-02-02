#pragma once
#include "util.h"
#include "entity.h"

#define CHECK_T_IS_EVENT static_assert(std::is_base_of<IEvent<T>, T>::value, "T is not of type Event");


template <class T>
struct IEvent {
	static const type_hash EventTypeID;
};


template <class T>
const type_hash IEvent<T>::EventTypeID = util::GetTypeHash<T>();


struct EntityCreatedEvent : public IEvent<EntityCreatedEvent> {
	Entity entity;
};

struct EntityDestroyedEvent : public IEvent<EntityDestroyedEvent> {
	Entity entity;
};

struct ComponentAddedEvent : public IEvent<ComponentAddedEvent> {
	Entity entity;
	type_hash componentType;
};

struct ComponentRemovedEvent : public IEvent<ComponentRemovedEvent> {
	Entity entity;
	type_hash componentType;
};