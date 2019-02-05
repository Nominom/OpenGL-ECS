#pragma once
#include <stdint.h>
#include <memory>

constexpr uint32_t ENTITY_NULL_ID = 0;

struct Entity {
	uint32_t ID = ENTITY_NULL_ID;

	inline bool operator ==(const Entity &b) const {
		return ID == b.ID;
	}
};

struct EntityArray {
	size_t size = 0;
	std::shared_ptr<Entity[]> data;

	inline Entity operator [](size_t index) const{
		return data[index];
	}

	inline Entity* begin() {
		return &data[0];
	}

	inline Entity* end() {
		return begin() + size;
	}

	inline const Entity* begin() const{
		return &data[0];
	}

	inline const Entity* end() const{
		return begin() + size;
	}
};

namespace std {

	template <>
	struct hash<Entity> {
		std::size_t operator()(const Entity& e) const {
			static std::hash<uint32_t> hasher;
			return hasher(e.ID);
		}
	};

}