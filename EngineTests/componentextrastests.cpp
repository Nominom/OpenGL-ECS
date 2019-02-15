#include "pch.h"

TEST(ComponentFilters, Filter) {

	TestSharedComponent1 shared1;
	TestSharedComponent2 shared2;

	EntityArchetype archetype_all =
		EntityArchetype(ComponentType::Get<TestComponent1>())
		.AddComponent(ComponentType::Get<TestComponent2>())
		.AddSharedComponent(&shared1)
		.AddSharedComponent(&shared2);

	EntityArchetype archetype_components =
		EntityArchetype(ComponentType::Get<TestComponent1>())
		.AddComponent(ComponentType::Get<TestComponent2>());

	EntityArchetype archetype_sharedcomponents =
		EntityArchetype()
		.AddSharedComponent(&shared1)
		.AddSharedComponent(&shared2);
	
	ComponentFilter filter1;
	filter1.Include<TestComponent1>().Include<TestComponent2>()
		.Include<TestSharedComponent1>().Include<TestSharedComponent2>();

	ComponentFilter filter2;
	filter2.Include<TestComponent1>();

	ComponentFilter filter3;
	filter3.Include<TestComponent2>().Exclude<TestSharedComponent1>();

	ComponentFilter filter4;
	filter4.Include<TestSharedComponent1>().Exclude<TestComponent1>();

	ASSERT_TRUE(filter1.Matches(archetype_all));
	ASSERT_FALSE(filter1.Matches(archetype_components));
	ASSERT_FALSE(filter1.Matches(archetype_sharedcomponents));

	ASSERT_TRUE(filter2.Matches(archetype_all));
	ASSERT_TRUE(filter2.Matches(archetype_components));
	ASSERT_FALSE(filter2.Matches(archetype_sharedcomponents));

	ASSERT_FALSE(filter3.Matches(archetype_all));
	ASSERT_TRUE(filter3.Matches(archetype_components));
	ASSERT_FALSE(filter3.Matches(archetype_sharedcomponents));

	ASSERT_FALSE(filter4.Matches(archetype_all));
	ASSERT_FALSE(filter4.Matches(archetype_components));
	ASSERT_TRUE(filter4.Matches(archetype_sharedcomponents));
}

TEST(ComponentDatablock, Components) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();

	TestSharedComponent1 shared1;
	TestSharedComponent2 shared2;

	EntityArchetype archetype_all =
		EntityArchetype(ComponentType::Get<TestComponent1>())
		.AddComponent(ComponentType::Get<TestComponent2>())
		.AddSharedComponent(&shared1)
		.AddSharedComponent(&shared2);

	EntityArray arr = entitymanager->CreateEntities(20);

	ComponentMemoryBlock block;
	block.Initialize(archetype_all);
	for (Entity e : arr) {
		block.AddEntity(e);
	}

	ComponentDatablock<TestComponent1, TestComponent2> datablock(&block);

	int i = 1;
	for (TestComponent1 &c : datablock.Get<TestComponent1>()) {
		c.testValue = i++;
	}

	i = 1;
	for (Entity e : arr) {
		ASSERT_EQ(block.GetComponent<TestComponent1>(e).testValue, i);
		i++;
	}
}

TEST(ComponentDatablock, Entities) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();

	TestSharedComponent1 shared1;
	TestSharedComponent2 shared2;

	EntityArchetype archetype_all =
		EntityArchetype(ComponentType::Get<TestComponent1>())
		.AddComponent(ComponentType::Get<TestComponent2>())
		.AddSharedComponent(&shared1)
		.AddSharedComponent(&shared2);

	EntityArray arr = entitymanager->CreateEntities(20);

	ComponentMemoryBlock block;
	block.Initialize(archetype_all);
	for (Entity e : arr) {
		block.AddEntity(e);
	}

	ComponentDatablock<TestComponent1, TestComponent2> datablock(&block);

	EntityIterator eit = datablock.GetEntities();
	for (int i = 0; i < 20; i++) {
		ASSERT_EQ(eit[i].ID, arr[i].ID);
	}
}

TEST(ComponentDatablock, SharedComponents) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();

	TestSharedComponent1 shared1;
	TestSharedComponent2 shared2;
	shared1.testInt = 1;
	shared2.testInt = 2;

	EntityArchetype archetype_all =
		EntityArchetype(ComponentType::Get<TestComponent1>())
		.AddComponent(ComponentType::Get<TestComponent2>())
		.AddSharedComponent(&shared1)
		.AddSharedComponent(&shared2);

	EntityArray arr = entitymanager->CreateEntities(10);

	ComponentMemoryBlock block;
	block.Initialize(archetype_all);
	for (Entity e : arr) {
		block.AddEntity(e);
	}

	ComponentDatablock<TestSharedComponent1, TestSharedComponent2> datablock(&block);

	ASSERT_EQ(datablock.Get<TestSharedComponent1>().component->testInt, shared1.testInt);
	ASSERT_EQ(datablock.Get<TestSharedComponent2>().component->testInt, shared2.testInt);
}