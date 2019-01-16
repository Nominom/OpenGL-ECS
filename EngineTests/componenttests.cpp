#include "pch.h"

TEST(Components, TypeID) {
	//Components
	ASSERT_NE(TestComponent1::ComponentTypeID, 0) << "TypeID is zero";
	ASSERT_NE(TestComponent2::ComponentTypeID, 0) << "TypeID is zero";

	ASSERT_NE(TestComponent1::ComponentTypeID, TestComponent2::ComponentTypeID) << "Both typeID's were the same";

	//Shared components
	ASSERT_NE(TestComponent1::ComponentTypeID, 0) << "Shared TypeID is zero";
	ASSERT_NE(TestSharedComponent2::ComponentTypeID, 0) << "Shared TypeID is zero";

	ASSERT_NE(TestSharedComponent1::ComponentTypeID, TestSharedComponent2::ComponentTypeID) << "Both shared typeID's were the same";
}

TEST(Components, Add) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();


	Entity entity1 = entitymanager->CreateEntity();
	TestComponent1 &testComponent = componentmanager->AddComponent<TestComponent1>(entity1);

	EXPECT_TRUE(componentmanager->HasComponent<TestComponent1>(entity1)) << "Entity didn't have component";
	EXPECT_FALSE(componentmanager->HasComponent<TestComponent2>(entity1)) << "Entity had extra component";
}

TEST(Components, Modify) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();


	Entity entity1 = entitymanager->CreateEntity();
	TestComponent1 &testComponent = componentmanager->AddComponent<TestComponent1>(entity1);

	ASSERT_EQ(testComponent.testValue, 0) << "Value is non-zero";

	EXPECT_TRUE(componentmanager->HasComponent<TestComponent1>(entity1)) << "Entity didn't have component";

	testComponent.testValue = 5;

	TestComponent1 &component = componentmanager->GetComponent<TestComponent1>(entity1);

	ASSERT_EQ(testComponent.testValue, component.testValue) << "Value wasn't modified";

	TestComponent2 &tc2 = componentmanager->AddComponent<TestComponent2>(entity1);
	
	ASSERT_EQ(5, componentmanager->GetComponent<TestComponent1>(entity1).testValue) << "value changed after adding another component";


}


TEST(Components, Remove) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();


	Entity entity1 = entitymanager->CreateEntity();
	TestComponent1 &testComponent = componentmanager->AddComponent<TestComponent1>(entity1);

	ASSERT_TRUE(componentmanager->HasComponent<TestComponent1>(entity1)) << "Entity didn't have component";

	componentmanager->RemoveComponent<TestComponent1>(entity1);

	ASSERT_FALSE(componentmanager->HasComponent<TestComponent1>(entity1)) << "Entity had component after removing";
}

TEST(Components, AddToMany) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();
	const size_t numents = 30000;
	EntityArray arr = entitymanager->CreateEntitites(numents);

	size_t i = 1;
	for (Entity e : arr) {
		componentmanager->AddComponent<TestComponent2>(e).testBigint = i++;
	}

	for (Entity e : arr) {
		componentmanager->AddComponent<TestComponent1>(e).testValue = i++;
	}

	for (Entity e : arr) {
		auto testVal = componentmanager->GetComponent<TestComponent1>(e).testValue;
		auto testBigInt = componentmanager->GetComponent<TestComponent2>(e).testBigint;

		ASSERT_GT(testVal, numents);
		ASSERT_NE(0, testBigInt);
	}
}