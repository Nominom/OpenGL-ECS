#pragma once
#include <vector>
#include "component.h"
#include "entityarchetypes.h"

namespace gleng {

	/*
	All types are in the same vector for efficiency.
	The order is = {includes..., excludes..., shared includes..., shared excludes...}
	*/
	struct ComponentQuery {
		std::vector<type_hash> types;
		size_t includes = 0;
		size_t excludes = 0;
		size_t shared_includes = 0;
		size_t shared_excludes = 0;

		inline bool Matches(const EntityArchetype &archetype) const{
			int i = 0;
			//Loop over includes
			size_t includesEnd = includes;
			for (i; i < includesEnd; i++) {
				if (!archetype.HasComponentType(types[i])) {
					return false;
				}
			}
			//Loop over excludes
			size_t excludesEnd = includesEnd + excludes;
			for (i; i < excludesEnd; i++) {
				if (archetype.HasComponentType(types[i])) {
					return false;
				}
			}

			//Loop over shared includes
			size_t sharedIncludesEnd = excludesEnd + shared_includes;
			for (i; i < sharedIncludesEnd; i++) {
				if (!archetype.HasSharedComponentType(types[i])) {
					return false;
				}
			}

			//Loop over shared excludes
			size_t sharedExcludesEnd = sharedIncludesEnd + shared_excludes;
			for (i; i < sharedExcludesEnd; i++) {
				if (archetype.HasSharedComponentType(types[i])) {
					return false;
				}
			}

			return true;
		}
	};


	namespace util {

		template <class T>
		struct IncludeType {

			template <class Q = T, std::enable_if_t<std::is_base_of<IComponent<Q>, Q>::value, int> = 0 >
			static ComponentQuery& Add(ComponentQuery& query) {
				type_hash type = IComponent<T>::ComponentTypeID;
				query.types.insert(query.types.begin(), type);
				query.includes++;
				return query;
			}

			template <class Q = T, std::enable_if_t<std::is_base_of<ISharedComponent<Q>, Q>::value, int> = 0>
			static ComponentQuery& Add(ComponentQuery& query) {
				type_hash type = IComponent<T>::ComponentTypeID;
				query.types.insert(query.types.begin() + query.includes + query.excludes, type);
				query.shared_includes++;
				return query;
			}
		};
		template <class T>
		struct ExcludeType {
			template <class Q = T, std::enable_if_t<std::is_base_of<IComponent<Q>, Q>::value, int> = 0 >
			static ComponentQuery& Add(ComponentQuery& query) {
				type_hash type = IComponent<T>::ComponentTypeID;
				query.types.insert(query.types.begin() + query.includes, type);
				query.excludes++;
				return query;
			}

			template <class Q = T, std::enable_if_t<std::is_base_of<ISharedComponent<Q>, Q>::value, int> = 0>
			static ComponentQuery& Add(ComponentQuery& query) {
				type_hash type = IComponent<T>::ComponentTypeID;
				query.types.insert(query.types.begin() + query.includes + query.excludes + query.shared_includes, type);
				query.shared_excludes++;
				return query;
			}
		};

		template <class ...Types>
		struct ComponentQueryTemplate {

			template <class ...Args>
			constexpr ComponentQueryTemplate<Types..., IncludeType<Args>...> Include() const {
				return ComponentQueryTemplate<Types..., IncludeType<Args>...>();
			}

			template <class ...Args>
			constexpr ComponentQueryTemplate<Types..., ExcludeType<Args>...> Exclude() const {
				return ComponentQueryTemplate<Types..., ExcludeType<Args>...>();
			}

			inline ComponentQuery Build() const {
				ComponentQuery q;
				q.types.reserve(sizeof...(Types));
				auto _ = { Types::Add(q)... };
				return q;
			}
		};
	}

	
	

	class ComponentQueryBuilder {
	public:
		template <class ...Args>
		constexpr util::ComponentQueryTemplate<util::IncludeType<Args>...> Include() const{
			return util::ComponentQueryTemplate<util::IncludeType<Args>...>();
		}
	};
}