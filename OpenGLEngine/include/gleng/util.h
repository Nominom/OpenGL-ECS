#pragma once
#include <functional>
#include <string>


//#define NDEBUG
#include <cassert>


#define KB(x)   ((size_t) (x) << 10)

typedef size_t type_hash;

namespace gleng {

	namespace util {

		template <class T>
		type_hash GetTypeHash() {
			static type_hash typeId = typeid(T).hash_code();
			return typeId;
		}


		struct typehasher {
			::std::size_t operator()(const type_hash hash) const {
				return hash;
			}
		};
	}
}
/*namespace std {

	template <>
	struct hash<type_hash> {
		std::size_t operator()(const type_hash hash) const {
			return hash;
		}
	};

}*/