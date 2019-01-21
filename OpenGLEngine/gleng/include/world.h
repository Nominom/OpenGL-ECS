#pragma once
#include "entitymanager.h"
#include "componentmanager.h"
#include "system.h"
#include "systemmanager.h"

class World {
public:
	static EntityManager* GetEntityManager(){
		static EntityManager manager;
		return &manager;
	}

	static ComponentManager* GetComponentManager() {
		static ComponentManager manager;
		return &manager;
	}

	static SystemManager* GetSystemManager() {
		static SystemManager manager;
		return &manager;
	}

	static void Setup(){
		GetEntityManager()->Clear();
		GetComponentManager()->Clear();
		GetSystemManager()->Clear();
		MemoryBlockAllocator::instance().Clear();
	}
};