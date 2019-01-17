#pragma once
#include "entity.h"
#include "component.h"
#include "entityarchetypes.h"


struct MemoryPtr {
	void* ptr;
	size_t size;
};
/*
class ComponentMemoryBlock {
private:
	size_t _size;
	size_t _maxSize;
public:
	static const size_t datasize = KB(16);
	uint8_t data[datasize];
	std::unordered_map<Entity, size_t> entityIdx;
	std::unordered_map<type_hash, MemoryPtr> dataLocations;

	ComponentMemoryBlock() = default;
	void Initialize(const EntityArchetype &type);

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
	inline T& GetComponent(const Entity &e) {
		CHECK_T_IS_COMPONENT;
		assert(entityIdx.find(e) != entityIdx.end());
		size_t idx = entityIdx[e];
		return GetComponentArray<T>()[idx];
	}

	size_t AddEntity(const Entity &e);
	void RemoveEntity(const Entity &e);
	size_t MoveEntityTo(const Entity &e, ComponentMemoryBlock *memblock);

	inline bool HasEntity(const Entity &e) const {
		return entityIdx.find(e) != entityIdx.end();
	}

	inline size_t GetEntityIndex(const Entity& e) {
		auto eidx = entityIdx.find(e);
		assert(eidx != entityIdx.end());
		return eidx->second;
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
};*/


class ComponentMemoryBlock {
private:
	size_t _size;
	size_t _maxSize;
public:
	static const size_t datasize = KB(16);
	uint8_t data[datasize];
	std::unordered_map<type_hash, MemoryPtr> dataLocations;

	ComponentMemoryBlock() = default;
	void Initialize(const EntityArchetype &type);

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

	~MemoryBlockAllocator();
};