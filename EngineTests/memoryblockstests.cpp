#include "pch.h"



TEST(ComponentMemoryBlock, Create) {
	EntityArchetype archetype12 = EntityArchetype() +
		ComponentType::Get<TestComponent1>() +
		ComponentType::Get<TestComponent2>();

	ComponentMemoryBlock memblock;
	memblock.Initialize(archetype12);

	ASSERT_EQ(memblock.size(), 0);

	size_t expectedSize = floor((float)ComponentMemoryBlock::datasize / (float)(sizeof(Entity) + ComponentType::Get<TestComponent1>().memorySize + ComponentType::Get<TestComponent2>().memorySize));

	ASSERT_EQ(memblock.maxSize(), expectedSize);

	Entity *earr = memblock.GetEntityArray();
	TestComponent1 *tc1arr = memblock.GetComponentArray<TestComponent1>();
	TestComponent2 *tc2arr = memblock.GetComponentArray<TestComponent2>();

	ASSERT_NE((void*)earr, (void*)tc1arr);
	ASSERT_NE((void*)earr, (void*)tc2arr);
	ASSERT_NE((void*)tc1arr, (void*)tc2arr);

	ASSERT_NE(earr, nullptr);
	ASSERT_NE(tc1arr, nullptr);
	ASSERT_NE(tc2arr, nullptr);
}

TEST(ComponentMemoryBlock, AddEntity) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();

	EntityArchetype archetype12 = EntityArchetype() +
		ComponentType::Get<TestComponent1>() +
		ComponentType::Get<TestComponent2>();

	ComponentMemoryBlock memblock;
	memblock.Initialize(archetype12);

	Entity e1 = entitymanager->CreateEntity();
	Entity e2 = entitymanager->CreateEntity();

	ASSERT_FALSE(memblock.HasEntity(e1));
	ASSERT_FALSE(memblock.HasEntity(e2));

	size_t e1idx = memblock.AddEntity(e1); // Add one entity

	ASSERT_EQ(e1idx, 0);

	ASSERT_TRUE(memblock.HasEntity(e1));
	ASSERT_FALSE(memblock.HasEntity(e2));

	size_t e2idx = memblock.AddEntity(e2); // Add second entity

	ASSERT_EQ(e2idx, 1);

	ASSERT_TRUE(memblock.HasEntity(e1));
	ASSERT_TRUE(memblock.HasEntity(e2));

	ASSERT_EQ(e1idx, memblock.GetEntityIndex(e1));
	ASSERT_EQ(e2idx, memblock.GetEntityIndex(e2));

	Entity *ents = memblock.GetEntityArray();
	ASSERT_EQ(ents[0], e1);
	ASSERT_EQ(ents[1], e2);

	ASSERT_EQ(memblock.size(), 2);
}

TEST(ComponentMemoryBlock, RemoveEntityFromMiddle) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();

	EntityArchetype archetype12 = EntityArchetype() +
		ComponentType::Get<TestComponent1>() +
		ComponentType::Get<TestComponent2>();

	ComponentMemoryBlock memblock;
	memblock.Initialize(archetype12);

	Entity e1 = entitymanager->CreateEntity();
	Entity e2 = entitymanager->CreateEntity();

	size_t e1idx = memblock.AddEntity(e1); // Add one entity
	size_t e2idx = memblock.AddEntity(e2); // Add second entity

	const int test_val = 6;

	memblock.GetComponent<TestComponent1>(e2).testValue = test_val;

	ASSERT_EQ(e1idx, 0);
	ASSERT_EQ(e2idx, 1);

	ASSERT_EQ(test_val, memblock.GetComponentArray<TestComponent1>()[e2idx].testValue);

	memblock.RemoveEntity(e1);

	ASSERT_FALSE(memblock.HasEntity(e1));

	e2idx = memblock.GetEntityIndex(e2);

	ASSERT_EQ(e2idx, 0);
	//check that component moved with the entity
	ASSERT_EQ(test_val, memblock.GetComponentArray<TestComponent1>()[e2idx].testValue);
	//check that memory was reset
	ASSERT_EQ(0, memblock.GetComponentArray<TestComponent1>()[1].testValue);

	ASSERT_EQ(memblock.GetEntityArray()[0], e2);
}

TEST(ComponentMemoryBlock, RemoveLastEntity) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();

	EntityArchetype archetype12 = EntityArchetype() +
		ComponentType::Get<TestComponent1>() +
		ComponentType::Get<TestComponent2>();

	ComponentMemoryBlock memblock;
	memblock.Initialize(archetype12);

	Entity e1 = entitymanager->CreateEntity();
	Entity e2 = entitymanager->CreateEntity();

	size_t e1idx = memblock.AddEntity(e1); // Add one entity
	size_t e2idx = memblock.AddEntity(e2); // Add second entity

	memblock.GetComponent<TestComponent1>(e1).testValue = 2;
	memblock.GetComponent<TestComponent1>(e2).testValue = 6;

	auto carr = memblock.GetComponentArray<TestComponent1>();
	auto ents = memblock.GetEntityArray();

	ASSERT_EQ(ents[0], e1);
	ASSERT_EQ(ents[1], e2);

	ASSERT_EQ(carr[0].testValue, 2);
	ASSERT_EQ(carr[1].testValue, 6);

	memblock.RemoveEntity(e2);

	ASSERT_EQ(ents[0], e1);
	ASSERT_EQ(ents[1], Entity());

	ASSERT_EQ(carr[0].testValue, 2);
	ASSERT_EQ(carr[1].testValue, 0);
}

TEST(ComponentMemoryBlock, RemoveOnlyEntity) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();

	EntityArchetype archetype12 = EntityArchetype() +
		ComponentType::Get<TestComponent1>() +
		ComponentType::Get<TestComponent2>();

	ComponentMemoryBlock memblock;
	memblock.Initialize(archetype12);

	Entity e1 = entitymanager->CreateEntity();
	size_t e1idx = memblock.AddEntity(e1); // Add one entity

	ASSERT_EQ(memblock.size(), 1);

	memblock.GetComponent<TestComponent1>(e1).testValue = 2;

	auto carr = memblock.GetComponentArray<TestComponent1>();
	auto ents = memblock.GetEntityArray();

	ASSERT_EQ(carr[0].testValue, 2);
	ASSERT_EQ(ents[0], e1);

	ASSERT_TRUE(memblock.HasEntity(e1));

	memblock.RemoveEntity(e1);

	ASSERT_FALSE(memblock.HasEntity(e1));

	ASSERT_EQ(carr[0].testValue, 0);
	ASSERT_EQ(ents[0], Entity());

	ASSERT_EQ(memblock.size(), 0);
}

TEST(ComponentMemoryBlock, MoveEntity) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();

	EntityArchetype archetype12 = EntityArchetype() +
		ComponentType::Get<TestComponent1>() +
		ComponentType::Get<TestComponent2>();

	ComponentMemoryBlock memblock1;
	memblock1.Initialize(archetype12);

	EntityArchetype archetype2 = EntityArchetype() +
		ComponentType::Get<TestComponent2>();

	ComponentMemoryBlock memblock2;
	memblock2.Initialize(archetype2);

	Entity e1 = entitymanager->CreateEntity();

	memblock1.AddEntity(e1);
	memblock1.GetComponent<TestComponent2>(e1).testBigint = 999;
	memblock1.GetComponent<TestComponent2>(e1).testFloat = 0.5f;

	memblock1.GetComponent<TestComponent1>(e1).testValue = 60;

	auto ctarr1 = memblock1.GetComponentArray<TestComponent1>();
	auto carr1 = memblock1.GetComponentArray<TestComponent2>();
	auto ents1 = memblock1.GetEntityArray();

	ASSERT_TRUE(memblock1.HasEntity(e1));
	ASSERT_EQ(ents1[0], e1);
	ASSERT_EQ(ctarr1[0].testValue, 60);
	ASSERT_EQ(carr1[0].testBigint, 999);
	ASSERT_EQ(carr1[0].testFloat, 0.5f);
	ASSERT_EQ(memblock1.size(), 1);

	memblock1.MoveEntityTo(e1, &memblock2);

	auto carr2 = memblock2.GetComponentArray<TestComponent2>();
	auto ents2 = memblock2.GetEntityArray();


	ASSERT_FALSE(memblock1.HasEntity(e1));
	ASSERT_EQ(ents1[0], Entity());
	ASSERT_EQ(ctarr1[0].testValue, 0);
	ASSERT_EQ(carr1[0].testBigint, 0);
	ASSERT_EQ(carr1[0].testFloat, 0.0f);
	ASSERT_EQ(memblock1.size(), 0);

	ASSERT_TRUE(memblock2.HasEntity(e1));
	ASSERT_EQ(ents2[0], e1);
	ASSERT_EQ(carr2[0].testBigint, 999);
	ASSERT_EQ(carr2[0].testFloat, 0.5f);
	ASSERT_EQ(memblock2.size(), 1);
}

TEST(ComponentMemoryBlock, MoveMany) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();

	EntityArchetype archetype12 = EntityArchetype() +
		ComponentType::Get<TestComponent1>() +
		ComponentType::Get<TestComponent2>();

	ComponentMemoryBlock memblock1;
	memblock1.Initialize(archetype12);

	EntityArchetype archetype2 = EntityArchetype() +
		ComponentType::Get<TestComponent2>();

	ComponentMemoryBlock memblock2;
	memblock2.Initialize(archetype2);

	const size_t numents = 20;

	EntityArray arr = entitymanager->CreateEntitites(numents);
	int i = 0;
	for (Entity e : arr) {
		ASSERT_EQ(i, memblock1.AddEntity(e));
		memblock1.GetComponent<TestComponent2>(e).testBigint = i;
		i++;
	}
	i = 0;
	for (Entity e : arr) {
		ASSERT_EQ(i, memblock1.MoveEntityTo(e, &memblock2));
		ASSERT_EQ(i, memblock2.GetComponent<TestComponent2>(e).testBigint);
		i++;
	}
}