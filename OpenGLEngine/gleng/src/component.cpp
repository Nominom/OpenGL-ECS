#include "..\include\component.h"
#include "..\include\entityarchetypes.h"


bool ComponentFilter::Matches(const EntityArchetype& archetype) const {
	for (type_hash t : includeTypes) {
		if (!archetype.HasComponentType(t)) {
			return false;
		}
	}

	for (type_hash t : excludeTypes) {
		if (archetype.HasComponentType(t)) {
			return false;
		}
	}

	return true;
}