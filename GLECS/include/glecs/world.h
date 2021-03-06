#pragma once
#include "entitymanager.h"
#include "componentmanager.h"
#include "systemmanager.h"
#include "eventmanager.h"
#include "componentquery.h"

namespace gleng {

	class World {
	public:
		static EntityManager* GetEntityManager() {
			static EntityManager manager(GetComponentManager(), GetEventManager());
			return &manager;
		}

		static ComponentManager* GetComponentManager() {
			static ComponentManager manager(GetEventManager());
			return &manager;
		}

		static SystemManager* GetSystemManager() {
			static SystemManager manager;
			return &manager;
		}

		static EventManager* GetEventManager() {
			static EventManager manager;
			return &manager;
		}

		static WorldAccessor GetWorldAccessor() {
			return WorldAccessor(GetEntityManager(), GetComponentManager(), GetEventManager());
		}

		static void Setup() {
			GetEntityManager()->Clear();
			GetComponentManager()->Clear();
			GetSystemManager()->Clear();
			GetEventManager()->Clear();
		}
	};

}