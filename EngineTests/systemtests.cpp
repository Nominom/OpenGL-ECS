#include "pch.h"



class TestSystem : public IComponentSystem<TestComponent1, TestComponent2> {
public:
	virtual void DoWork(double deltaTime, const ComponentDatablock<TestComponent1, TestComponent2> &components) override {
		ComponentDataIterator<TestComponent1> data1= components.Get<TestComponent1>();
		ComponentDataIterator<TestComponent2> data2 = components.Get<TestComponent2>();
		EntityIterator entities = components.GetEntities();

		for (size_t i = 0; i < components.size(); i++) {
			data1[i].testValue++;
			data2[i].testBigint++;
			data2[i].testFloat += deltaTime;
			auto id = entities[i].ID;
		}
	}

};

class TestSystem2 : public IComponentSystem<TestComponent1> {
public:
	virtual void DoWork(double deltaTime, const ComponentDatablock<TestComponent1> &components) override {
		ComponentDataIterator<TestComponent1> data1 = components.Get<TestComponent1>();

		for (size_t i = 0; i < components.size(); i++) {
			data1[i].testValue++;
		}
	}

};


class TestSystem3 : public IComponentSystem<TestComponent1, TestSharedComponent1> {
public:
	virtual void DoWork(double deltaTime, const ComponentDatablock<TestComponent1, TestSharedComponent1> &components) override {
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
	virtual void DoWork(double deltaTime, const ComponentDatablock<TestComponent1> &components) override {

		ComponentDataIterator<TestComponent1> data1 = components.Get<TestComponent1>();

		for (size_t i = 0; i < components.size(); i++) {
			data1[i].testValue++;
		}
	}

	virtual inline ComponentQuery GetQuery() override{
		ComponentQuery filter = ComponentQueryBuilder().Include<TestComponent1>()
			.Exclude<TestComponent2, TestSharedComponent2>().Build();
		return filter;
	}
};

class BeforeWorkAfterWorkTestSystem : public IComponentSystem<TestComponent1> {

public:

	std::vector<int> types;

	virtual void DoWork(double deltaTime, const ComponentDatablock<TestComponent1> &components) override {
		ComponentDataIterator<TestComponent1> data1 = components.Get<TestComponent1>();
		types.push_back(1);
	}

	void BeforeWork(double deltaTime, const WorldAccessor& world) override{
		types.push_back(0);
	}

	void AfterWork(double deltaTime, const WorldAccessor& world) override {
		types.push_back(2);
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

	ComponentDatablock<TestComponent1, TestComponent2> datablock(&memblock);

	TestSystem system;
	
	system.DoWork(0.5, datablock);

	ASSERT_EQ(memblock.GetComponent<TestComponent1>(e1).testValue, 1);
	ASSERT_EQ(memblock.GetComponent<TestComponent2>(e1).testBigint, 1);
	ASSERT_EQ(memblock.GetComponent<TestComponent2>(e1).testFloat, 0.5f);

	ASSERT_EQ(memblock.GetComponent<TestComponent1>(e2).testValue, 1);
	ASSERT_EQ(memblock.GetComponent<TestComponent2>(e2).testBigint, 1);
	ASSERT_EQ(memblock.GetComponent<TestComponent2>(e2).testFloat, 0.5f);


	ASSERT_EQ(memblock.GetComponent<TestComponent1>(e3).testValue, 1);
	ASSERT_EQ(memblock.GetComponent<TestComponent2>(e3).testBigint, 1);
	ASSERT_EQ(memblock.GetComponent<TestComponent2>(e3).testFloat, 0.5f);


	ASSERT_EQ(memblock.GetComponent<TestComponent1>(e4).testValue, 1);
	ASSERT_EQ(memblock.GetComponent<TestComponent2>(e4).testBigint, 1);
	ASSERT_EQ(memblock.GetComponent<TestComponent2>(e4).testFloat, 0.5f);


}


TEST(ComponentSystems, RegisterSystem) {

	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();
	SystemManager *systemmanager = World::GetSystemManager();

	EntityArchetype archetype =
		EntityArchetype(ComponentType::Get<TestComponent1>())
		.AddComponent(ComponentType::Get<TestComponent2>());

	

	EntityArray arr = entitymanager->CreateEntities(1000);

	for (Entity e : arr) {
		componentmanager->MoveToArchetype(e, archetype);
	}

	systemmanager->RegisterSystem(new TestSystem());

	const int numUpdates = 10;

	for (int i = 0; i < numUpdates; i++) {
		systemmanager->Update(World::GetWorldAccessor(), 0.5);
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



	EntityArray arr12 = entitymanager->CreateEntities(1000);

	for (Entity e : arr12) {
		componentmanager->MoveToArchetype(e, archetype12);
	}

	EntityArray arr1 = entitymanager->CreateEntities(1000);

	for (Entity e : arr1) {
		componentmanager->MoveToArchetype(e, archetype1);
	}

	systemmanager->RegisterSystem(new TestSystem());
	systemmanager->RegisterSystem(new TestSystem2());

	const int numUpdates = 10;

	for (int i = 0; i < numUpdates; i++) {
		systemmanager->Update(World::GetWorldAccessor(), 0.5);
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

	EntityArray arr = entitymanager->CreateEntities(4000);

	for (Entity e : arr) {
		componentmanager->MoveToArchetype(e, archetype);
	}

	systemmanager->RegisterSystem(new TestSystem3());

	const int numUpdates = 100;

	for (int i = 0; i < numUpdates; i++) {
		systemmanager->Update(World::GetWorldAccessor(), 0.5);
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

	 TestSharedComponent2 shared2;
	 TestSharedComponent1 shared1;
	 shared2.testInt = 0;
	 shared1.testInt = 0;

	 EntityArchetype archetype12 =
		 EntityArchetype(ComponentType::Get<TestComponent1>())
		 .AddComponent(ComponentType::Get<TestComponent2>());

	 EntityArchetype archetype2 =
		 EntityArchetype(ComponentType::Get<TestComponent1>()).AddSharedComponent(&shared2);

	 EntityArchetype archetype3 =
		 EntityArchetype(ComponentType::Get<TestComponent1>()).AddSharedComponent(&shared1);

	 EntityArchetype archetype1 =
		 EntityArchetype(ComponentType::Get<TestComponent1>());

	 const size_t numents = 1000;

	 EntityArray arr12 = entitymanager->CreateEntities(numents, archetype12);

	 EntityArray arr1 = entitymanager->CreateEntities(numents, archetype1);

	 EntityArray arr2 = entitymanager->CreateEntities(numents, archetype2);

	 EntityArray arr3 = entitymanager->CreateEntities(numents, archetype3);


	 systemmanager->RegisterSystem(new TestSystem4());

	 const int numUpdates = 10;

	 for (int i = 0; i < numUpdates; i++) {
		 systemmanager->Update(World::GetWorldAccessor(), 0.5);
	 }

	 for (Entity e : arr12) {
		 ASSERT_EQ(componentmanager->GetComponent<TestComponent1>(e).testValue, 0);
	 }

	 for (Entity e : arr1) {
		 ASSERT_EQ(componentmanager->GetComponent<TestComponent1>(e).testValue, numUpdates);
	 }

	 for (Entity e : arr2) {
		 ASSERT_EQ(componentmanager->GetComponent<TestComponent1>(e).testValue, 0);
	 }

	 for (Entity e : arr3) {
		 ASSERT_EQ(componentmanager->GetComponent<TestComponent1>(e).testValue, numUpdates);
	 }
 }


 TEST(ComponentSystems, BeforeAfterWork) {

	 World::Setup();
	 EntityManager *entitymanager = World::GetEntityManager();
	 ComponentManager *componentmanager = World::GetComponentManager();
	 SystemManager *systemmanager = World::GetSystemManager();

	 EntityArchetype archetype =
		 EntityArchetype(ComponentType::Get<TestComponent1>());

	 Entity e = entitymanager->CreateEntity(archetype);

	 BeforeWorkAfterWorkTestSystem *system = new BeforeWorkAfterWorkTestSystem();

	 systemmanager->RegisterSystem(system);
	 
	 systemmanager->Update(World::GetWorldAccessor(), 1);

	
	 ASSERT_EQ(system->types.size(), 3);
	 ASSERT_EQ(system->types[0], 0);
	 ASSERT_EQ(system->types[1], 1);
	 ASSERT_EQ(system->types[2], 2);
 }


class TestISystem1 : public ISystem {
public:
	double totalDeltaTime = 0;


	void Update(double deltaTime, const WorldAccessor& world) override {
		totalDeltaTime += deltaTime;
	}
	
};

class TestISystem2 : public ISystem {
public:
	std::vector<ComponentDatablock<TestComponent1, TestSharedComponent1>> data;

	void Update(double deltaTime, const WorldAccessor& world) override {
		world.GetComponentData(data);

		for(auto block : data) {
			ComponentDataIterator<TestComponent1> testC1 = block.Get<TestComponent1>();
			ComponentDataIterator<TestSharedComponent1> testSC1 = block.Get<TestSharedComponent1>();

			size_t len = block.size();
			for (size_t i = 0; i < len; i++) {
				testC1[i].testValue++;
			}
		}
	}

};


TEST(Systems, Update) {
	World::Setup();
	SystemManager *systemmanager = World::GetSystemManager();


	TestISystem1 *system = new TestISystem1();

	systemmanager->RegisterSystem(system);

	systemmanager->Update(World::GetWorldAccessor(), 1);

	ASSERT_DOUBLE_EQ(system->totalDeltaTime, 1);
}


TEST(Systems, DataAccess) {

	World::Setup();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();
	SystemManager *systemmanager = World::GetSystemManager();

	TestISystem2 *system = new TestISystem2();

	systemmanager->RegisterSystem(system);

	TestSharedComponent1 shared1;
	shared1.testInt = 0;

	EntityArchetype archetype = EntityArchetype::Create<TestComponent1>(&shared1);

	const size_t numents = 10000;
	const size_t numUpdates = 1000;

	EntityArray arr = entitymanager->CreateEntities(numents, archetype);

	for (size_t i = 0; i < numUpdates; ++i) {
		systemmanager->Update(World::GetWorldAccessor(), 1);
	}

	for (Entity e : arr) {
		ASSERT_EQ(componentmanager->GetComponent<TestComponent1>(e).testValue, numUpdates);
	}
}