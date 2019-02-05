#pragma once
#include "entity.h"
#include "component.h"
#include "entityarchetypes.h"

#ifndef ECS_NO_TSL
#include "tsl/robin_map.h"
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
	void Initialize(const EntityArchetype &);

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

	size_t AddEntity(const Entity &e);
	//returns the last entity that was moved in place of eidx
	Entity RemoveEntityMoveLast(size_t eidx);
	size_t CopyEntityTo(size_t eidx, const Entity& e, ComponentMemoryBlock *memblock);

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

	~MemoryBlockAllocator();
};

struct SharedComponentMemory {
	type_hash type;
	size_t memorySize;
	void* memoryLocation;
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

		T* ptr = new T;

		memory.memoryLocation = ptr;

		sharedComponents.emplace(memory.type, memory);
		return ptr;
	}


	static SharedComponentAllocator& instance() {
		static SharedComponentAllocator instance;
		return instance;
	}

	SharedComponentAllocator(SharedComponentAllocator const&) = delete;
	void operator=(SharedComponentAllocator const&) = delete;

	~SharedComponentAllocator();
};