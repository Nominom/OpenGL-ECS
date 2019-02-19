#pragma once
#include "eventmanager.h"
#include "component.h"

namespace gleng {


	struct IComponentEventSpawnerInstance {
		virtual void ComponentAdded(const Entity&, EventManager*) = 0;
		virtual void ComponentRemoved(const Entity&, EventManager*) = 0;
	};

	template <class T>
	struct ComponentEventSpawnerInstance : public IComponentEventSpawnerInstance {

		inline void ComponentAdded(const Entity& e, EventManager* em) {
			ComponentAddedEvent<T> event;
			event.entity = e;
			em->QueueEvent(event);
		}

		inline void ComponentRemoved(const Entity& e, EventManager* em) {
			ComponentRemovedEvent<T> event;
			event.entity = e;
			em->QueueEvent(event);
		}
	};

	class ComponentEventSpawner {
#ifdef ECS_NO_TSL
		std::unordered_map<type_hash, IComponentEventSpawnerInstance*, util::typehasher> componentEventSpawners;
#else
		tsl::robin_map<type_hash, IComponentEventSpawnerInstance*, util::typehasher> componentEventSpawners;
#endif

		ComponentEventSpawner() = default;
	public:
		template <class T>
		inline void RegisterEventSpawnerForComponent() {
			CHECK_T_IS_COMPONENT;
			type_hash type = IComponent<T>::ComponentTypeID;
			if (componentEventSpawners.find(type) == componentEventSpawners.end()) {
				ComponentEventSpawnerInstance<T> *cesi = new ComponentEventSpawnerInstance<T>();
				componentEventSpawners.emplace(type, cesi);
			}
		}

		inline void ComponentAdded(type_hash componentType, const Entity& entity, EventManager* em) {
			auto found = componentEventSpawners.find(componentType);
			if (found != componentEventSpawners.end()) {
				found->second->ComponentAdded(entity, em);
			}
		}

		inline void ComponentRemoved(type_hash componentType, const Entity& entity, EventManager* em) {
			auto found = componentEventSpawners.find(componentType);
			if (found != componentEventSpawners.end()) {
				found->second->ComponentRemoved(entity, em);
			}
		}

		template <class T>
		inline void ComponentAdded(const Entity& entity, EventManager* em) {
			CHECK_T_IS_COMPONENT;
			type_hash componentType = IComponent<T>::ComponentTypeID;
			auto found = componentEventSpawners.find(componentType);
			if (found != componentEventSpawners.end()) {
				static_cast<ComponentEventSpawnerInstance<T>*>(found->second)->ComponentAdded(entity, em);
			}
		}

		template <class T>
		inline void ComponentRemoved(const Entity& entity, EventManager* em) {
			CHECK_T_IS_COMPONENT;
			type_hash componentType = IComponent<T>::ComponentTypeID;
			auto found = componentEventSpawners.find(componentType);
			if (found != componentEventSpawners.end()) {
				static_cast<ComponentEventSpawnerInstance<T>*>(found->second)->ComponentRemoved(entity, em);
			}

		}

		static ComponentEventSpawner& instance() {
			static ComponentEventSpawner instance;
			return instance;
		}

		ComponentEventSpawner(ComponentEventSpawner const&) = delete;
		void operator=(ComponentEventSpawner const&) = delete;

		inline ~ComponentEventSpawner() {
			for (auto keyval : componentEventSpawners) {
				delete(keyval.second);
			}

			componentEventSpawners.clear();
		}
	};

}