#include "..\include\entityarchetypes.h"
#include <functional>

void EntityArchetype::GenerateHash() {
	type_hash finalHash = 0;
	for (auto hash : componentTypesMemory) {
		finalHash xor_eq hash.first;
	}
	std::hash<size_t> hasher;
	for (auto pair : sharedComponents) {
		size_t valueHash = hasher(pair.second);
		finalHash xor_eq pair.first;
		finalHash xor_eq valueHash;
	}
	_archetypeHash = finalHash;
}
