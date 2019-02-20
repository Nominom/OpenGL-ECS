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


	struct ISharedComponentEventSpawnerInstance {
		virtual void SharedComponentAdded(const Entity&, void*, EventManager*) = 0;
		virtual void SharedComponentRemoved(const Entity&, void*, EventManager*) = 0;
};

	template <class T>
	struct SharedComponentEventSpawnerInstance : public ISharedComponentEventSpawnerInstance {

		inline void SharedComponentAdded(const Entity& e, void* c, EventManager* em) {
			SharedComponentAddedEvent<T> event;
			event.entity = e;
			event.component = static_cast<T*>(c);
			em->QueueEvent(event);
		}

		inline void SharedComponentRemoved(const Entity& e, void* c, EventManager* em) {
			SharedComponentRemovedEvent<T> event;
			event.entity = e;
			event.component = static_cast<T*>(c);
			em->QueueEvent(event);
		}
	};

	class ComponentEventSpawner {
#ifdef ECS_NO_TSL
		std::unordered_map<type_hash, IComponentEventSpawnerInstance*, util::typehasher> componentEventSpawners;
#else
		tsl::robin_map<type_hash, IComponentEventSpawnerInstance*, util::typehasher> componentEventSpawners;
		tsl::robin_map<type_hash, ISharedComponentEventSpawnerInstance*, util::typehasher> sharedComponentEventSpawners;

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

		template <class T>
		inline void RegisterEventSpawnerForSharedComponent() {
			CHECK_T_IS_SHARED_COMPONENT;
			type_hash type = ISharedComponent<T>::ComponentTypeID;
			if (sharedComponentEventSpawners.find(type) == sharedComponentEventSpawners.end()) {
				SharedComponentEventSpawnerInstance<T> *cesi = new SharedComponentEventSpawnerInstance<T>();
				sharedComponentEventSpawners.emplace(type, cesi);
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


		inline void SharedComponentAdded(type_hash componentType, const Entity& entity, void* component, EventManager* em) {
			auto found = sharedComponentEventSpawners.find(componentType);
			if (found != sharedComponentEventSpawners.end()) {
				found->second->SharedComponentAdded(entity, component, em);
			}
		}

		inline void SharedComponentRemoved(type_hash componentType, const Entity& entity, void* component, EventManager* em) {
			auto found = sharedComponentEventSpawners.find(componentType);
			if (found != sharedComponentEventSpawners.end()) {
				found->second->SharedComponentRemoved(entity, component, em);
			}
		}

		template <class T>
		inline void SharedComponentAdded(const Entity& entity, T* component, EventManager* em) {
			CHECK_T_IS_SHARED_COMPONENT;
			type_hash componentType = ISharedComponent<T>::ComponentTypeID;
			auto found = sharedComponentEventSpawners.find(componentType);
			if (found != sharedComponentEventSpawners.end()) {
				static_cast<SharedComponentEventSpawnerInstance<T>*>(found->second)
					->SharedComponentAdded(entity, component, em);
			}
		}

		template <class T>
		inline void SharedComponentRemoved(const Entity& entity, T* component, EventManager* em) {
			CHECK_T_IS_SHARED_COMPONENT;
			type_hash componentType = ISharedComponent<T>::ComponentTypeID;
			auto found = sharedComponentEventSpawners.find(componentType);
			if (found != sharedComponentEventSpawners.end()) {
				static_cast<SharedComponentEventSpawnerInstance<T>*>(found->second)
					->SharedComponentRemoved(entity, component, em);
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

			for (auto keyval : sharedComponentEventSpawners) {
				delete(keyval.second);
			}

			componentEventSpawners.clear();
			sharedComponentEventSpawners.clear();
		}
	};

}