#pragma once
#include "component.h"
#include <vector>
#include "entity.h"
#include "memoryblocks.h"

namespace gleng {

	template <class T>
	struct Optional{};


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

	template <typename T>
	struct ComponentDataIterator < Optional<T>, typename std::enable_if<std::is_base_of<IComponent<T>, T>::value>::type>{
		bool isAvailable;
		T* data;
		size_t len;

		inline ComponentDataIterator(ComponentMemoryBlock *block) {
			CHECK_T_IS_COMPONENT;
			if (block->type.HasComponentType(IComponent<T>::ComponentTypeID)) {
				len = block->size();
				data = block->GetComponentArray<T>();
				isAvailable = true;
			} else {
				len = 0;
				data = nullptr;
				isAvailable = false;
			}
		}
	};

	template <typename T>
	struct ComponentDataIterator < Optional<T>, typename std::enable_if<std::is_base_of<ISharedComponent<T>, T>::value>::type> {
		bool isAvailable;
		T* component;

		inline ComponentDataIterator(ComponentMemoryBlock *block) {
			CHECK_T_IS_SHARED_COMPONENT;

			if (block->type.HasSharedComponentType(ISharedComponent<T>::ComponentTypeID)) {
				component = block->type.GetSharedComponent<T>();
				isAvailable = true;
			} else {
				component = nullptr;
				isAvailable = false;
			}
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
}