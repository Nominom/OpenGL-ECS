#pragma once
#include "componentmanager.h"
#include "worldaccessor.h"
#include "componentquery.h"
#include "componentdatablock.h"

namespace gleng {


	template <class ...Components>
	class IComponentSystem {
	public:
		bool running = true;
		virtual void DoWork(double deltaTime, const ComponentDatablock<Components...>&) = 0;
		virtual inline void BeforeWork(double deltaTime, const WorldAccessor& world) {}
		virtual inline void AfterWork(double deltaTime, const WorldAccessor& world) {}



		virtual inline ComponentQuery GetQuery() {
			return ComponentQueryBuilder().Include<Components...>().Build();
		}
	};

	class ISystem {
	public:
		bool running = true;
		virtual void Update(double deltaTime, const WorldAccessor& world) {}
	};
}