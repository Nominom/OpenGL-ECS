#include "pch.h"

TEST(SharedComponents, Create) {
	World::Setup();
	ComponentManager *componentmanager = World::GetComponentManager();


	TestSharedComponent1 *testComponent = componentmanager->CreateSharedComponent<TestSharedComponent1>();

	ASSERT_NE(testComponent, nullptr);

	TestSharedComponent1 *testComponent2 = componentmanager->CreateSharedComponent<TestSharedComponent1>();

	ASSERT_NE(testComponent2, nullptr);

	ASSERT_NE(testComponent, testComponent2);
}

TEST(SharedComponents, Add) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	Entity entity1 = entitymanager->CreateEntity();

	ASSERT_FALSE(componentmanager->HasSharedComponent<TestSharedComponent1>(entity1));

	TestSharedComponent1 *testComponent = componentmanager->CreateSharedComponent<TestSharedComponent1>();

	componentmanager->AddSharedComponent(entity1, testComponent);

	testComponent->testInt = 999;

	ASSERT_TRUE(componentmanager->HasSharedComponent<TestSharedComponent1>(entity1));

	ASSERT_EQ(componentmanager->GetSharedComponent<TestSharedComponent1>(entity1)->testInt, 999);
	
}

TEST(SharedComponents, Remove) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	Entity entity1 = entitymanager->CreateEntity();

	ASSERT_FALSE(componentmanager->HasSharedComponent<TestSharedComponent1>(entity1));

	TestSharedComponent1 *testComponent = componentmanager->CreateSharedComponent<TestSharedComponent1>();

	componentmanager->AddSharedComponent(entity1, testComponent);

	ASSERT_TRUE(componentmanager->HasSharedComponent<TestSharedComponent1>(entity1));

	componentmanager->RemoveSharedComponent<TestSharedComponent1>(entity1);
}