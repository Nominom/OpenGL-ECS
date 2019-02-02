#pragma once
#include "system.h"

class IComponentSystemDataInjector {
public:
	virtual void ExecuteSystem(ComponentManager* componentManager, double deltaTime) = 0;
	virtual ~IComponentSystemDataInjector() = default;
};

template <class ...Args>
class SystemDataInjector : public IComponentSystemDataInjector {
private:
	IComponentSystem<Args...> *system;
	ComponentFilter filter;
public:
	inline virtual void ExecuteSystem(ComponentManager* componentManager, double deltaTime) {
		std::vector<ComponentMemoryBlock*> blocks = componentManager->GetMemoryBlocks(filter);
		for (ComponentMemoryBlock *block : blocks) {
			ComponentDatablock<Args...> datablock(block);
			system->DoWork(deltaTime, datablock);
		}
		system->Update(deltaTime);
	}

	inline SystemDataInjector(IComponentSystem<Args...> *s) {
		this->system = s;
		filter = s->GetFilter();
		//auto _ = { filter.Include<Args>()... };
	}

	inline virtual ~SystemDataInjector() {
		delete(system);
	}
};

class SystemManager {
private:
	std::list<IComponentSystemDataInjector*> systemInjectors;

public:
	template <class ...Args>
	inline void RegisterComponentSystem(IComponentSystem<Args...> *system) {
		systemInjectors.push_back(new SystemDataInjector<Args...>(system));
	}

	inline void Update(ComponentManager* cm, double deltaTime) {
		for (IComponentSystemDataInjector *system : systemInjectors) {
			system->ExecuteSystem(cm, deltaTime);
		}
	}

	inline ~SystemManager() {
		Clear();
	}

	inline void Clear() {
		for (IComponentSystemDataInjector *system : systemInjectors) {
			delete(system);
		}
		systemInjectors.clear();
	}
};