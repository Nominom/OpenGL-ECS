#include "..\include\memoryblocks.h"


void ComponentMemoryBlock::Initialize(const EntityArchetype & type) {
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

size_t ComponentMemoryBlock::AddEntity(const Entity & e) {
	assert(e.ID != ENTITY_NULL_ID);

	assert(_size < _maxSize);
	GetEntityArray()[_size] = e;
	return _size++; //Return old size and increment size by one 
}

Entity ComponentMemoryBlock::RemoveEntityMoveLast(size_t eidx) {
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

size_t ComponentMemoryBlock::CopyEntityTo(size_t eidx, const Entity& e, ComponentMemoryBlock *memblock) {
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


MemoryBlockAllocator::~MemoryBlockAllocator() {
	for (ComponentMemoryBlock* block : allBlocks) {
		delete(block);
	}

	allBlocks.clear();
}