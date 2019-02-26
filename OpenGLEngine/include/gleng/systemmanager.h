#pragma once
#include "system.h"
#include "worldaccessor.h"

namespace gleng {

	class ISystemExecutor {
	public:
		virtual void ExecuteSystem(const WorldAccessor& world, double deltaTime) = 0;
		virtual ~ISystemExecutor() = default;
	};

	template <class ...Args>
	class ComponentSystemExecutor : public ISystemExecutor {
	private:
		IComponentSystem<Args...> *system;
		ComponentFilter filter;
	public:
		inline virtual void ExecuteSystem(const WorldAccessor& world, double deltaTime) {
			system->BeforeWork(deltaTime, world);
			std::vector<ComponentMemoryBlock*> blocks = world.componentmanager->GetMemoryBlocks(filter);
			for (ComponentMemoryBlock *block : blocks) {
				ComponentDatablock<Args...> datablock(block);
				system->DoWork(deltaTime, datablock);
			}
			system->AfterWork(deltaTime, world);
		}

		inline ComponentSystemExecutor(IComponentSystem<Args...> *s) {
			this->system = s;
			filter = s->GetFilter();
			//auto _ = { filter.Include<Args>()... };
		}

		inline virtual ~ComponentSystemExecutor() {
			delete(system);
		}
	};

	class GenericSystemExecutor : public ISystemExecutor {
	private:
		ISystem *system;
		ComponentFilter filter;
	public:
		inline virtual void ExecuteSystem(const WorldAccessor& world, double deltaTime) {
			system->Update(deltaTime, world);
		}

		inline GenericSystemExecutor(ISystem *s) {
			this->system = s;
		}

		inline virtual ~GenericSystemExecutor() {
			delete(system);
		}
	};

	class SystemManager {
	private:
		std::vector<ISystemExecutor*> systemExecutors;

	public:
		template <class ...Args>
		inline void RegisterSystem(IComponentSystem<Args...> *system) {
			systemExecutors.push_back(new ComponentSystemExecutor<Args...>(system));
		}

		inline void RegisterSystem(ISystem *system) {
			systemExecutors.push_back(new GenericSystemExecutor(system));
		}

		inline void Update(const WorldAccessor& world, double deltaTime) {
			for (ISystemExecutor *system : systemExecutors) {
				system->ExecuteSystem(world, deltaTime);
			}
		}

		inline ~SystemManager() {
			Clear();
		}

		inline void Clear() {
			for (ISystemExecutor *system : systemExecutors) {
				delete(system);
			}
			systemExecutors.clear();
		}
	};

}