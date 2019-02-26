#pragma once
#include "entitymanager.h"

namespace gleng {
	class WorldAccessor {
	public:
		EntityManager * const entitymanager;
		ComponentManager * const componentmanager;
		EventManager * const eventmanager;

		inline WorldAccessor(EntityManager *em, ComponentManager *cm, EventManager *evm) :
			entitymanager(em), componentmanager(cm), eventmanager(evm) {}

		//TODO access componentgroup by query
	};
}
