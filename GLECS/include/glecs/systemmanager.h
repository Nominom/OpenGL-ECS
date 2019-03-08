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
		std::vector<ComponentDatablock<Args...>> data;
		ComponentQuery query;
	public:
		inline virtual void ExecuteSystem(const WorldAccessor& world, double deltaTime) {
			system->BeforeWork(deltaTime, world);

			world.GetComponentData(data, query);
			for (auto block : data) {
				system->DoWork(deltaTime, block);
			}
			system->AfterWork(deltaTime, world);
		}

		inline ComponentSystemExecutor(IComponentSystem<Args...> *s) {
			this->system = s;
			query = s->GetQuery();
		}

		inline virtual ~ComponentSystemExecutor() {
			delete(system);
		}
	};

	class GenericSystemExecutor : public ISystemExecutor {
	private:
		ISystem *system;

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