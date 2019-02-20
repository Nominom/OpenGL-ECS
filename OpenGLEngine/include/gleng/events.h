#pragma once
#include "util.h"
#include "entity.h"

#define CHECK_T_IS_EVENT static_assert(std::is_base_of<IEvent<T>, T>::value, "T is not of type Event");

namespace gleng {

	template <class T>
	struct IEvent {
		static const type_hash EventTypeID;
	};


	template <class T>
	const type_hash IEvent<T>::EventTypeID = util::GetTypeHash<T>();


	struct EntityCreatedEvent : public IEvent<EntityCreatedEvent> {
		Entity entity;

		EntityCreatedEvent() = default;
		inline EntityCreatedEvent(const Entity& e) {
			entity = e;
		}
	};

	struct EntityDestroyedEvent : public IEvent<EntityDestroyedEvent> {
		Entity entity;

		EntityDestroyedEvent() = default;
		inline EntityDestroyedEvent(const Entity& e) {
			entity = e;
		}
	};

	template <class T>
	struct ComponentAddedEvent : public IEvent<ComponentAddedEvent<T>> {
		Entity entity;
	};

	template <class T>
	struct ComponentRemovedEvent : public IEvent<ComponentRemovedEvent<T>> {
		Entity entity;
	};

	template <class T>
	struct SharedComponentAddedEvent : public IEvent<SharedComponentAddedEvent<T>> {
		Entity entity;
		T* component;
	};

	template <class T>
	struct SharedComponentRemovedEvent : public IEvent<SharedComponentRemovedEvent<T>> {
		Entity entity;
		T* component;
	};
}