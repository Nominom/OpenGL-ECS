#pragma once
#include "componentmanager.h"

namespace gleng {

	template <typename T, typename Enable = void>
	struct ComponentDataIterator;

	template <typename T>
	struct ComponentDataIterator<T, typename std::enable_if<std::is_base_of<ISharedComponent<T>, T>::value>::type> {

		T* const component;

		inline ComponentDataIterator(ComponentMemoryBlock *block) : component(block->type.GetSharedComponent<T>()) {
			CHECK_T_IS_SHARED_COMPONENT;
		}

	};

	template <typename T>
	struct ComponentDataIterator<T, typename std::enable_if<std::is_base_of<IComponent<T>, T>::value>::type> {
		T* data;
		const size_t len;

		inline ComponentDataIterator(ComponentMemoryBlock *block) : len(block->size()) {
			CHECK_T_IS_COMPONENT;
			data = block->GetComponentArray<T>();
		}

		inline T* begin() {
			return data;
		}

		inline T* end() {
			return data + len;
		}

		inline T& operator [](size_t index) {
			assert(index < len);
			return data[index];
		}
	};

	//TODO: const iterator. Doesn't mark as dirty



	struct EntityIterator {
		const Entity* data;
		const size_t len;

		inline EntityIterator(ComponentMemoryBlock *block) : len(block->size()) {
			data = block->GetEntityArray();
		}

		inline const Entity* begin() {
			return data;
		}

		inline const Entity* end() {
			return data + len;
		}

		inline const Entity& operator [](size_t index) {
			assert(index < len);
			return data[index];
		}
	};

	template <class ...Components>
	class ComponentDatablock {
	private:
		size_t len;
		std::tuple<ComponentDataIterator<Components>...> data;
		EntityIterator entities;
	public:

		inline ComponentDatablock(ComponentMemoryBlock *block) :
			data(ComponentDataIterator<Components>(block)...), entities(block) {
			len = block->size();
		}

		template <class T>
		inline ComponentDataIterator<T> Get() const {
			return std::get<ComponentDataIterator<T>>(data);
		}

		inline size_t size() const {
			return len;
		}

		inline EntityIterator GetEntities() const {
			return entities;
		}
	};

	template <class ...Components>
	class IComponentSystem {
	public:
		bool running = true;
		virtual void DoWork(double deltaTime, const ComponentDatablock<Components...>&) = 0;
		virtual inline void Update(double deltaTime) {}


		virtual inline ComponentFilter GetFilter() {
			ComponentFilter filter;
			auto _ = { filter.Include<Components>()... };
			return filter;
		}
	};


}