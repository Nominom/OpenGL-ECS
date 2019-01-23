#pragma once
#include "system.h"

class ISystemDataInjector {
public:
	virtual void ExecuteSystem(ComponentManager* componentManager, double deltaTime) = 0;
	virtual ~ISystemDataInjector() = default;
};

template <class ...Args>
class SystemDataInjector : public ISystemDataInjector {
private:
	IComponentSystem<Args...> *system;
	ComponentFilter filter;
public:
	inline virtual void ExecuteSystem(ComponentManager* componentManager, double deltaTime) {
		std::vector<ComponentMemoryBlock*> blocks = componentManager->GetMemoryBlocks(filter);
		for (ComponentMemoryBlock *block : blocks) {
			ComponentDataBlockArray<Args...> datablock(block);
			system->DoWork(datablock);
		}
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
	std::list<ISystemDataInjector*> systemInjectors;

public:
	template <class ...Args>
	inline void RegisterSystem(IComponentSystem<Args...> *system) {
		systemInjectors.push_back(new SystemDataInjector<Args...>(system));
	}

	inline void Update(ComponentManager* cm, double deltaTime) {
		for (ISystemDataInjector *system : systemInjectors) {
			system->ExecuteSystem(cm, deltaTime);
		}
	}

	inline ~SystemManager() {
		Clear();
	}

	inline void Clear() {
		for (ISystemDataInjector *system : systemInjectors) {
			delete(system);
		}
		systemInjectors.clear();
	}
};