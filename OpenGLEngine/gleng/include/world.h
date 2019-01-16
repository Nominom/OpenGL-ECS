#pragma once
#include "entitymanager.h"
#include "componentmanager.h"


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

	static void Setup(){}
};