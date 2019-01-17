#include "pch.h"

TEST(Entities, CreateSingles) {
	EntityManager manager;
	Entity entity1 = manager.CreateEntity();
	Entity entity2 = manager.CreateEntity();

	ASSERT_NE(entity1.ID, ENTITY_NULL_ID) << "Entity was null";
	ASSERT_NE(entity2.ID, ENTITY_NULL_ID) << "Entity was null";

	ASSERT_GT(entity2.ID, entity1.ID) << "Entity indexing was not working! " 
		<< entity1.ID << " " << entity2.ID;
}

TEST(Entities, CreateArrays) {
	EntityManager manager;

	const int entityArrSize = 100;
	EntityArray entArr = manager.CreateEntitites(entityArrSize);

	ASSERT_EQ(entArr.size, entityArrSize) << "Entity array size does not match the requested size";

	ASSERT_NE(entArr[0].ID, ENTITY_NULL_ID) << "Entity was null";

	for (size_t i = 1; i < entArr.size; i++) {
		ASSERT_NE(entArr[i].ID, entArr[i-1].ID) << "entity array had two same ID's";
		ASSERT_NE(entArr[1].ID, ENTITY_NULL_ID) << "Entity was null";
	}
}


TEST(EntityArchetypes, ComponentTypes) {
	ComponentType type1 = ComponentType::Get<TestComponent1>();
	ASSERT_EQ(type1.type, TestComponent1::ComponentTypeID);
	ASSERT_EQ(type1.memorySize, sizeof(TestComponent1));
	ASSERT_NE(type1.memorySize, 0);

	ComponentType type2 = ComponentType::Get<TestComponent2>();
	ASSERT_EQ(type2.type, TestComponent2::ComponentTypeID);
	ASSERT_EQ(type2.memorySize, sizeof(TestComponent2));
	ASSERT_NE(type2.memorySize, 0);
}


TEST(EntityArchetypes, Join) {
	EntityArchetype archetype;

	ASSERT_FALSE(archetype.HasComponentType(TestComponent1::ComponentTypeID));
	ASSERT_FALSE(archetype.HasComponentType(TestComponent2::ComponentTypeID));

	EntityArchetype archetype1 = archetype.AddComponent(ComponentType::Get<TestComponent1>());
	ASSERT_TRUE(archetype1.HasComponentType(TestComponent1::ComponentTypeID));
	ASSERT_FALSE(archetype1.HasComponentType(TestComponent2::ComponentTypeID));

	EntityArchetype archetype2 = archetype.AddComponent(ComponentType::Get<TestComponent2>());
	ASSERT_FALSE(archetype2.HasComponentType(TestComponent1::ComponentTypeID));
	ASSERT_TRUE(archetype2.HasComponentType(TestComponent2::ComponentTypeID));

	EntityArchetype archetype12 = archetype1.AddComponent(ComponentType::Get<TestComponent2>());
	ASSERT_TRUE(archetype12.HasComponentType(TestComponent1::ComponentTypeID));
	ASSERT_TRUE(archetype12.HasComponentType(TestComponent2::ComponentTypeID));

}

TEST(EntityArchetypes, Hash) {
	EntityArchetype archetype;

	EntityArchetype archetype1 = archetype.AddComponent(ComponentType::Get<TestComponent1>());

	ASSERT_NE(archetype1.ArchetypeHash(), 0);
	ASSERT_NE(archetype1.ArchetypeHash(), archetype.ArchetypeHash());


	EntityArchetype archetype2 = archetype.AddComponent(ComponentType::Get<TestComponent2>());

	ASSERT_NE(archetype2.ArchetypeHash(), 0);
	ASSERT_NE(archetype2.ArchetypeHash(), archetype.ArchetypeHash());
	ASSERT_NE(archetype2.ArchetypeHash(), archetype1.ArchetypeHash());

	EntityArchetype archetype12 = archetype1.AddComponent(ComponentType::Get<TestComponent2>());
	EntityArchetype archetype21 = archetype2.AddComponent(ComponentType::Get<TestComponent1>());

	ASSERT_EQ(archetype12.ArchetypeHash(), archetype21.ArchetypeHash());

	ASSERT_NE(archetype12.ArchetypeHash(), archetype1.ArchetypeHash());
	ASSERT_NE(archetype21.ArchetypeHash(), archetype1.ArchetypeHash());

	ASSERT_NE(archetype12.ArchetypeHash(), archetype2.ArchetypeHash());
	ASSERT_NE(archetype21.ArchetypeHash(), archetype2.ArchetypeHash());

}

TEST(EntityArchetypes, InitFromComponent) {
	ComponentType type = ComponentType::Get<TestComponent1>();
	EntityArchetype archetype(type);

	ASSERT_TRUE(archetype.HasComponentType(type.type));
	ASSERT_NE(archetype.ArchetypeHash(), 0);
	ASSERT_EQ(archetype.ArchetypeHash(), type.type);
}

TEST(EntityArchetypes, RemoveComponent) {
	ComponentType type = ComponentType::Get<TestComponent2>();

	EntityArchetype archetype(type);
	
	ASSERT_TRUE(archetype.HasComponentType(type.type));

	EntityArchetype newType = archetype.RemoveComponent(type);

	ASSERT_FALSE(newType.HasComponentType(type.type));
	ASSERT_EQ(newType.ArchetypeHash(), 0);
}

TEST(EntityArchetypes, RemoveXORHash) {
	ComponentType type1 = ComponentType::Get<TestComponent1>();
	ComponentType type2 = ComponentType::Get<TestComponent2>();


	EntityArchetype archetype(type1);
	archetype = archetype.AddComponent(type2);



	EntityArchetype newType = archetype.RemoveComponent(type2);

	ASSERT_EQ(archetype.ArchetypeHash() ^ type2.type, newType.ArchetypeHash());

	ASSERT_EQ(newType.ArchetypeHash(), type1.type);

	ASSERT_EQ(newType.ArchetypeHash() ^ type1.type, 0);
}