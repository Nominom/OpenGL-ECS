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
			auto id = entities[i].ID;
		}
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

	for (int i = 0; i < 10; i++) {
		systemmanager->Update(componentmanager, 0.5);
	}

	for (Entity e : arr) {
		ASSERT_EQ(componentmanager->GetComponent<TestComponent1>(e).testValue, 10);
		ASSERT_EQ(componentmanager->GetComponent<TestComponent2>(e).testBigint, 10);
	}
}
