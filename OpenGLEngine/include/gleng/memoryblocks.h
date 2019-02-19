#pragma once
#include "entity.h"
#include "component.h"
#include "entityarchetypes.h"

#ifndef ECS_NO_TSL
#include "../tsl/robin_map.h"
#endif


struct MemoryPtr {
	void* ptr;
	size_t size;
};


class ComponentMemoryBlock {
private:
	size_t _size = 0;
	size_t _maxSize = 0;
public:
	static const size_t datasize = KB(16);
	uint8_t data[datasize];
	EntityArchetype type;

#ifdef ECS_NO_TSL
	std::unordered_map<type_hash, MemoryPtr, util::typehasher> dataLocations;
#else
	tsl::robin_map<type_hash, MemoryPtr, util::typehasher> dataLocations;
#endif // ECS_NO_TSL

	ComponentMemoryBlock() = default;

	inline void Initialize(const EntityArchetype & type) {
		this->type = type;

		size_t componentSizeCombined = 0;

		//space for entity array
		componentSizeCombined += sizeof(Entity);

		for (auto t : type.GetComponentTypes()) {
			assert(t.second > 0);
			componentSizeCombined += t.second;
		}

		assert(componentSizeCombined > 0);

		_maxSize = floor((float)datasize / (float)componentSizeCombined);

		size_t nextLoc = _maxSize * sizeof(Entity);//Start from after entity array

		assert(sizeof(data[0]) == 1);
		assert(sizeof(data) == datasize);
		for (auto t : type.GetComponentTypes()) {
			MemoryPtr ptr;
			ptr.ptr = &data[nextLoc];
			ptr.size = t.second;
			dataLocations.emplace(t.first, ptr);
			nextLoc += _maxSize * t.second;
		}

		_size = 0;

		memset(data, 0, datasize);
	}

	inline Entity* GetEntityArray() {
		return reinterpret_cast<Entity*>(data);
	}

	template <class T>
	inline T* GetComponentArray() {
		CHECK_T_IS_COMPONENT;
		type_hash componentTypeID = IComponent<T>::ComponentTypeID;

		assert(dataLocations.find(componentTypeID) != dataLocations.end());
		MemoryPtr ptr = dataLocations[componentTypeID];

		return static_cast<T*>(ptr.ptr);
	}

	template <class T>
	inline T& GetComponent(size_t idx) {
		CHECK_T_IS_COMPONENT;
		assert(idx < _size);
		return GetComponentArray<T>()[idx];
	}

	template <class T>
	inline T& GetComponent(const Entity &e) {
		CHECK_T_IS_COMPONENT;
		size_t idx = GetEntityIndex(e);
		return GetComponent<T>(idx);
	}

	inline size_t AddEntity(const Entity & e) {
		assert(e.ID != ENTITY_NULL_ID);

		assert(_size < _maxSize);
		GetEntityArray()[_size] = e;
		return _size++; //Return old size and increment size by one 
	}

	//returns the last entity that was moved in place of eidx
	inline Entity RemoveEntityMoveLast(size_t eidx) {
		assert(eidx < _size);

		size_t idx = eidx;

		Entity* entArr = GetEntityArray();

		size_t lastIdx = _size - 1;
		if (idx != lastIdx) {
			//Move last entitys data in place of removed entity
			for (auto locations : dataLocations) {
				MemoryPtr mp = locations.second;
				size_t idxOffset = mp.size * idx;
				size_t lastIdxOffset = mp.size * lastIdx;

				memcpy(static_cast<char*>(mp.ptr) + idxOffset,
					(static_cast<char*>(mp.ptr) + lastIdxOffset), mp.size);//copy data from last to idx
				memset(static_cast<char*>(mp.ptr) + lastIdxOffset, 0, mp.size);//set data of last to zeroes
			}

			Entity e2 = entArr[lastIdx];
			entArr[idx] = entArr[lastIdx];
			entArr[lastIdx].ID = ENTITY_NULL_ID; //Change last to be null entity
		} else {
			for (auto locations : dataLocations) {
				MemoryPtr mp = locations.second;
				size_t lastIdxOffset = mp.size * lastIdx;
				memset(static_cast<char*>(mp.ptr) + lastIdxOffset, 0, mp.size);//set data of last to zeroes
			}
			entArr[lastIdx].ID = ENTITY_NULL_ID; //Change last to be null entity
		}

		_size--;
		if (_size == 0) {
			return Entity();
		} else {
			return entArr[idx];
		}
	}


	inline size_t CopyEntityTo(size_t eidx, const Entity& e, ComponentMemoryBlock *memblock) {
		if (eidx >= _size) {
			int num = 0;
		}
		assert(eidx < _size);
		assert(e.ID != ENTITY_NULL_ID);

		size_t newIdx = memblock->AddEntity(e); //Add entity to other memoryblock
		size_t oldIdx = eidx;

		//Move all data from src to dest
		for (auto keyval : memblock->dataLocations) {
			type_hash type = keyval.first;
			MemoryPtr dest = keyval.second;

			auto dataloc = dataLocations.find(type);
			if (dataloc != dataLocations.end()) {
				MemoryPtr src = dataloc->second;
				size_t destOffset = newIdx * dest.size;
				size_t srcOffset = oldIdx * src.size;
				size_t size = dest.size;

				memcpy(static_cast<char*>(dest.ptr) + destOffset,
					(static_cast<char*>(src.ptr) + srcOffset), size);//copy data from old to new
			}
		}
		return newIdx;
	}

	inline bool HasEntity(const Entity &e) const {
		const Entity* begin = reinterpret_cast<const Entity*>(data);
		const Entity* end = begin + _size;
		return std::find(begin, end, e) != end;
	}

	inline size_t GetEntityIndex(const Entity& e) {
		const Entity* begin = reinterpret_cast<const Entity*>(data);
		const Entity* end = begin + _size;
		auto eidx = std::find(begin, end, e);
		assert(eidx != end);
		return eidx - begin;
	}

	inline size_t size() {
		return _size;
	}

	inline size_t maxSize() {
		return _maxSize;
	}

	inline bool HasRoom() {
		return _size < _maxSize;
	}
	ComponentMemoryBlock & operator =(ComponentMemoryBlock&&) = delete;
	ComponentMemoryBlock & operator =(const ComponentMemoryBlock &) = delete;
	ComponentMemoryBlock(const ComponentMemoryBlock &) = delete;
};

class MemoryBlockAllocator {
private:
	std::vector<ComponentMemoryBlock*> allBlocks;
	MemoryBlockAllocator() = default;
public:
	ComponentMemoryBlock* Allocate() {
		ComponentMemoryBlock* newBlock = new ComponentMemoryBlock();

		allBlocks.push_back(newBlock);

		return newBlock;
	}

	ComponentMemoryBlock* Deallocate(ComponentMemoryBlock* block) {
		auto found = std::find(allBlocks.begin(), allBlocks.end(), block);
		if (found != allBlocks.end()) {
			delete(*found);
			allBlocks.erase(found);
		}
	}

	static MemoryBlockAllocator& instance(){
		static MemoryBlockAllocator instance;
		return instance;
	}

	MemoryBlockAllocator(MemoryBlockAllocator const&) = delete;
	void operator=(MemoryBlockAllocator const&) = delete;

	inline void Clear() {
		for (ComponentMemoryBlock* block : allBlocks) {
			delete(block);
		}
		allBlocks.clear();
	}

	inline ~MemoryBlockAllocator() {
		Clear();
	}
};

struct SharedComponentMemory {
	type_hash type;
	size_t memorySize;
	std::shared_ptr<void> memoryLocation;

	inline bool operator ==(const void* other) {
		return (memoryLocation.get()) == other;
	}
};

class SharedComponentAllocator {
private:
	std::unordered_multimap<type_hash, SharedComponentMemory> sharedComponents;

	SharedComponentAllocator() = default;
public:

	template<class T>
	inline T* Allocate() {
		CHECK_T_IS_SHARED_COMPONENT;
		SharedComponentMemory memory;
		memory.type = ISharedComponent<T>::ComponentTypeID;
		memory.memorySize = sizeof(T);

		std::shared_ptr<T> ptr = std::make_shared<T>();
		//T* ptr = new T;

		memory.memoryLocation = ptr;

		sharedComponents.emplace(memory.type, memory);
		return ptr.get();
	}

	template<class T>
	inline void Deallocate(T* component) {
		CHECK_T_IS_SHARED_COMPONENT;
		
		type_hash type = ISharedComponent<T>::ComponentTypeID;

		auto result = sharedComponents.equal_range(type);
		for (auto it = result.first; it != result.second; ++it) {
			if(it->second == component) {
				sharedComponents.erase(it->first);
				break;
			}
		}
	}


	static SharedComponentAllocator& instance() {
		static SharedComponentAllocator instance;
		return instance;
	}

	inline void Clear() {
		sharedComponents.clear();
	}

	SharedComponentAllocator(SharedComponentAllocator const&) = delete;
	void operator=(SharedComponentAllocator const&) = delete;

	inline ~SharedComponentAllocator() {
		Clear();
	}
};