#pragma once
#include "entitymanager.h"
#include "componentdatablock.h"

namespace gleng {
	class WorldAccessor {
	public:
		EntityManager * const entitymanager;
		ComponentManager * const componentmanager;
		EventManager * const eventmanager;

		inline WorldAccessor(EntityManager *em, ComponentManager *cm, EventManager *evm) :
			entitymanager(em), componentmanager(cm), eventmanager(evm) {}

		//TODO access componentgroup by query

		template <class ...Components>
		inline size_t GetComponentData(std::vector<ComponentDatablock<Components...>> &out_datablocks, const ComponentQuery& query) const {
			return componentmanager->GetComponentDataBlocks(out_datablocks, query);
		}

		template <class ...Components>
		inline size_t GetComponentData(std::vector<ComponentDatablock<Components...>> &out_datablocks) const{
			static ComponentQuery query = ComponentQueryBuilder().Include<Components...>().Build();
			return GetComponentData(out_datablocks, query);
		}
	};
}
