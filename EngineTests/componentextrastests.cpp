#include "pch.h"

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

TEST(ComponentDatablock, Optional) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();


	EntityArchetype archetype12 =
		EntityArchetype(ComponentType::Get<TestComponent1>())
		.AddComponent(ComponentType::Get<TestComponent2>());

	EntityArray arr12 = entitymanager->CreateEntities(20);
	ComponentMemoryBlock block12;
	block12.Initialize(archetype12);
	for (Entity e : arr12) {
		block12.AddEntity(e);
	}

	EntityArchetype archetype1 = EntityArchetype(ComponentType::Get<TestComponent1>());
	EntityArray arr1 = entitymanager->CreateEntities(20);
	ComponentMemoryBlock block1;
	block1.Initialize(archetype1);
	for (Entity e : arr1) {
		block1.AddEntity(e);
	}

	ComponentDatablock<TestComponent1, Optional<TestComponent2>> datablock12(&block12);
	ComponentDatablock<TestComponent1, Optional<TestComponent2>> datablock1(&block1);

	ASSERT_TRUE(datablock12.Get<Optional<TestComponent2>>().isAvailable);
	ASSERT_FALSE(datablock1.Get<Optional<TestComponent2>>().isAvailable);

	ASSERT_EQ(datablock12.Get<Optional<TestComponent2>>().len, 20);
	ASSERT_EQ(datablock1.Get<Optional<TestComponent2>>().len, 0);

	ASSERT_NE(datablock12.Get<Optional<TestComponent2>>().data, nullptr);
	ASSERT_EQ(datablock1.Get<Optional<TestComponent2>>().data, nullptr);


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