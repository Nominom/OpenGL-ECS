#include "pch.h"



class TestSystem : public IComponentSystem<TestComponent1, TestComponent2> {
public:
	virtual void DoWork(const ComponentDataBlockArray<TestComponent1, TestComponent2> &components) {
		ComponentDataIterator<TestComponent1> data1= components.Get<TestComponent1>();
		ComponentDataIterator<TestComponent2> data2 = components.Get<TestComponent2>();
		EntityIterator entities = components.GetEntities();

		for (size_t i = 0; i < components.size(); i++) {
			data1[i].testValue++;
			data2[i].testBigint++;
			data2.begin();
			//data2.GetSharedComponent();
			auto id = entities[i].ID;
		}
	}

};

class TestSystem2 : public IComponentSystem<TestComponent1> {
public:
	virtual void DoWork(const ComponentDataBlockArray<TestComponent1> &components) {
		ComponentDataIterator<TestComponent1> data1 = components.Get<TestComponent1>();

		for (size_t i = 0; i < components.size(); i++) {
			data1[i].testValue++;
		}
	}

};


class TestSystem3 : public IComponentSystem<TestComponent1, TestSharedComponent1> {
public:
	virtual void DoWork(const ComponentDataBlockArray<TestComponent1, TestSharedComponent1> &components) {
		ComponentDataIterator<TestComponent1> data1 = components.Get<TestComponent1>();
		ComponentDataIterator<TestSharedComponent1> data2 = components.Get<TestSharedComponent1>();

		data2.component->testInt++;
		
		for (size_t i = 0; i < components.size(); i++) {
			data1[i].testValue++;
		}
	}

};

class TestSystem4 : public IComponentSystem<TestComponent1> {
public:
	virtual void DoWork(const ComponentDataBlockArray<TestComponent1> &components) {

		ComponentDataIterator<TestComponent1> data1 = components.Get<TestComponent1>();

		for (size_t i = 0; i < components.size(); i++) {
			data1[i].testValue++;
		}
	}

	virtual inline ComponentFilter GetFilter() {
		ComponentFilter filter;
		filter.Include<TestComponent1>();
		filter.Exclude<TestComponent2>();
		return filter;
	}
};


TEST(ComponentSystems, DoWork) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();
	EntityArchetype archetype = 
		EntityArchetype(ComponentType::Get<TestComponent1>())
		.AddComponent(ComponentType::Get<TestComponent2>());

	ComponentMemoryBlock memblock;
	memblock.Initialize(archetype);

	Entity e1 = entitymanager->CreateEntity();
	Entity e2 = entitymanager->CreateEntity();
	Entity e3 = entitymanager->CreateEntity();
	Entity e4 = entitymanager->CreateEntity();


	memblock.AddEntity(e1);
	memblock.AddEntity(e2);
	memblock.AddEntity(e3);
	memblock.AddEntity(e4);

	ComponentDataBlockArray<TestComponent1, TestComponent2> datablock(&memblock);

	TestSystem system;
	
	system.DoWork(datablock);

	ASSERT_EQ(memblock.GetComponent<TestComponent1>(e1).testValue, 1);
	ASSERT_EQ(memblock.GetComponent<TestComponent2>(e1).testBigint, 1);

	ASSERT_EQ(memblock.GetComponent<TestComponent1>(e2).testValue, 1);
	ASSERT_EQ(memblock.GetComponent<TestComponent2>(e2).testBigint, 1);

	ASSERT_EQ(memblock.GetComponent<TestComponent1>(e3).testValue, 1);
	ASSERT_EQ(memblock.GetComponent<TestComponent2>(e3).testBigint, 1);

	ASSERT_EQ(memblock.GetComponent<TestComponent1>(e4).testValue, 1);
	ASSERT_EQ(memblock.GetComponent<TestComponent2>(e4).testBigint, 1);

}


TEST(ComponentSystems, RegisterSystem) {

	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();
	SystemManager *systemmanager = World::GetSystemManager();

	EntityArchetype archetype =
		EntityArchetype(ComponentType::Get<TestComponent1>())
		.AddComponent(ComponentType::Get<TestComponent2>());

	

	EntityArray arr = entitymanager->CreateEntitites(1000);

	for (Entity e : arr) {
		componentmanager->MoveToArchetype(e, archetype);
	}

	systemmanager->RegisterSystem(new TestSystem());

	const int numUpdates = 10;

	for (int i = 0; i < numUpdates; i++) {
		systemmanager->Update(componentmanager, 0.5);
	}

	for (Entity e : arr) {
		ASSERT_EQ(componentmanager->GetComponent<TestComponent1>(e).testValue, numUpdates);
		ASSERT_EQ(componentmanager->GetComponent<TestComponent2>(e).testBigint, numUpdates);
	}
}


TEST(ComponentSystems, MultipleSystems) {
	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();
	SystemManager *systemmanager = World::GetSystemManager();

	EntityArchetype archetype12 =
		EntityArchetype(ComponentType::Get<TestComponent1>())
		.AddComponent(ComponentType::Get<TestComponent2>());

	EntityArchetype archetype1 =
		EntityArchetype(ComponentType::Get<TestComponent1>());



	EntityArray arr12 = entitymanager->CreateEntitites(1000);

	for (Entity e : arr12) {
		componentmanager->MoveToArchetype(e, archetype12);
	}

	EntityArray arr1 = entitymanager->CreateEntitites(1000);

	for (Entity e : arr1) {
		componentmanager->MoveToArchetype(e, archetype1);
	}

	systemmanager->RegisterSystem(new TestSystem());
	systemmanager->RegisterSystem(new TestSystem2());

	const int numUpdates = 10;

	for (int i = 0; i < numUpdates; i++) {
		systemmanager->Update(componentmanager, 0.5);
	}

	for (Entity e : arr12) {
		ASSERT_EQ(componentmanager->GetComponent<TestComponent1>(e).testValue, numUpdates * 2);
		ASSERT_EQ(componentmanager->GetComponent<TestComponent2>(e).testBigint, numUpdates);
	}

	for (Entity e : arr1) {
		ASSERT_EQ(componentmanager->GetComponent<TestComponent1>(e).testValue, numUpdates);
	}
	
}


 TEST(ComponentSystems, SharedComponentSystem) {

	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();
	SystemManager *systemmanager = World::GetSystemManager();


	TestSharedComponent1 shared1;

	shared1.testInt = 0;

	EntityArchetype archetype =
		EntityArchetype(ComponentType::Get<TestComponent1>()).AddSharedComponent(&shared1);

	EntityArray arr = entitymanager->CreateEntitites(5000);

	for (Entity e : arr) {
		componentmanager->MoveToArchetype(e, archetype);
	}

	systemmanager->RegisterSystem(new TestSystem3());

	const int numUpdates = 10;

	for (int i = 0; i < numUpdates; i++) {
		systemmanager->Update(componentmanager, 0.5);
	}

	for (Entity e : arr) {
		ASSERT_EQ(componentmanager->GetComponent<TestComponent1>(e).testValue, numUpdates);
		ASSERT_NE(componentmanager->GetSharedComponent<TestSharedComponent1>(e)->testInt, 0);
	}

	ASSERT_NE(shared1.testInt, 0);
}

 TEST(ComponentSystems, CustomFilter) {
	 World::Setup();
	 EntityManager *entitymanager = World::GetEntityManager();
	 ComponentManager *componentmanager = World::GetComponentManager();
	 SystemManager *systemmanager = World::GetSystemManager();

	 EntityArchetype archetype12 =
		 EntityArchetype(ComponentType::Get<TestComponent1>())
		 .AddComponent(ComponentType::Get<TestComponent2>());

	 EntityArchetype archetype1 =
		 EntityArchetype(ComponentType::Get<TestComponent1>());



	 EntityArray arr12 = entitymanager->CreateEntitites(200);

	 for (Entity e : arr12) {
		 componentmanager->MoveToArchetype(e, archetype12);
	 }

	 EntityArray arr1 = entitymanager->CreateEntitites(200);

	 for (Entity e : arr1) {
		 componentmanager->MoveToArchetype(e, archetype1);
	 }

	 systemmanager->RegisterSystem(new TestSystem4());

	 const int numUpdates = 10;

	 for (int i = 0; i < numUpdates; i++) {
		 systemmanager->Update(componentmanager, 0.5);
	 }

	 for (Entity e : arr12) {
		 ASSERT_EQ(componentmanager->GetComponent<TestComponent1>(e).testValue, 0);
	 }

	 for (Entity e : arr1) {
		 ASSERT_EQ(componentmanager->GetComponent<TestComponent1>(e).testValue, numUpdates);
	 }
 }