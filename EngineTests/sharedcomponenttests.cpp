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


TEST(SharedComponents, Destructor) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	TestSharedComponentWithDestructor::numDestructions = 0;

	TestSharedComponentWithDestructor *component = componentmanager->CreateSharedComponent<TestSharedComponentWithDestructor>();
	TestSharedComponentWithDestructor *component1 = componentmanager->CreateSharedComponent<TestSharedComponentWithDestructor>();
	TestSharedComponentWithDestructor *component2 = componentmanager->CreateSharedComponent<TestSharedComponentWithDestructor>();


	componentmanager->DestroySharedComponent(component);
	componentmanager->DestroySharedComponent(component1);
	componentmanager->DestroySharedComponent(component2);
	/*
	SharedComponentAllocator::instance().Deallocate(component);
	SharedComponentAllocator::instance().Deallocate(component1);
	SharedComponentAllocator::instance().Deallocate(component2);*/


	ASSERT_EQ(TestSharedComponentWithDestructor::numDestructions, 3);


	TestSharedComponentWithDestructor::numDestructions = 0;


	const size_t numComponents = 100;

	for (int i = 0; i < numComponents; ++i) {
		TestSharedComponentWithDestructor *component = componentmanager->CreateSharedComponent<TestSharedComponentWithDestructor>();
	}

	World::Setup();

	ASSERT_EQ(TestSharedComponentWithDestructor::numDestructions, numComponents);
}