#include "pch.h"
#include <chrono>

class TestEvent1Listener : public IEventListener<TestEvent1> {
public:
	int sumOfEvents = 0;
	unsigned long long  sumOfValues = 0;

	void ProcessEvents(const EventIterator<TestEvent1> &eventIterator) override{
		for (const TestEvent1 &event : eventIterator) {
			++sumOfEvents;
			sumOfValues += event.testInt;
		}
	}
};


class EntityCreatedEventListener : public IEventListener<EntityCreatedEvent> {
public:
	std::vector<Entity> entities;
	int sumOfEntities = 0;

	void ProcessEvents(const EventIterator<EntityCreatedEvent> &eventIterator) override {
		for (const EntityCreatedEvent &event : eventIterator) {
			++sumOfEntities;
			entities.push_back(event.entity);
		}
	}
};

class EntityDestroyedEventListener : public IEventListener<EntityDestroyedEvent> {
public:
	std::vector<Entity> entities;
	int sumOfEntities = 0;

	void ProcessEvents(const EventIterator<EntityDestroyedEvent> &eventIterator) override {
		for (const EntityDestroyedEvent &event : eventIterator) {
			++sumOfEntities;
			entities.push_back(event.entity);
		}
	}
};

class ComponentAddedEventListener : public IEventListener<ComponentAddedEvent<TestComponent1>> {
public:
	std::vector<Entity> entities;
	int sumOfEvents = 0;

	virtual void ProcessEvents(const EventIterator<ComponentAddedEvent<TestComponent1>> &eventIterator) override {
		for (const ComponentAddedEvent<TestComponent1> &event : eventIterator) {
			++sumOfEvents;
			entities.push_back(event.entity);
		}
	}
};

class ComponentRemovedEventListener : public IEventListener<ComponentRemovedEvent<TestComponent1>> {
public:
	std::vector<Entity> entities;
	int sumOfEvents = 0;

	virtual void ProcessEvents(const EventIterator<ComponentRemovedEvent<TestComponent1>> &eventIterator) override{
		for (const ComponentRemovedEvent<TestComponent1> &event : eventIterator) {
			++sumOfEvents;
			entities.push_back(event.entity);
		}
	}
};

class SharedComponentAddedEventListener : public IEventListener<SharedComponentAddedEvent<TestSharedComponent1>> {
public:
	std::vector<Entity> entities;
	std::vector<TestSharedComponent1*> components;
	int sumOfEvents = 0;

	virtual void ProcessEvents(const EventIterator<SharedComponentAddedEvent<TestSharedComponent1>> &eventIterator) override {
		for (const SharedComponentAddedEvent<TestSharedComponent1> &event : eventIterator) {
			++sumOfEvents;
			entities.push_back(event.entity);
			components.push_back(event.component);
		}
	}
};

class SharedComponentRemovedEventListener : public IEventListener<SharedComponentRemovedEvent<TestSharedComponent1>> {
public:
	std::vector<Entity> entities;
	std::vector<TestSharedComponent1*> components;
	int sumOfEvents = 0;

	virtual void ProcessEvents(const EventIterator<SharedComponentRemovedEvent<TestSharedComponent1>> &eventIterator) override {
		for (const SharedComponentRemovedEvent<TestSharedComponent1> &event : eventIterator) {
			++sumOfEvents;
			entities.push_back(event.entity);
			components.push_back(event.component);
		}
	}
};

TEST(Events, TypeID) {

	ASSERT_NE(ComponentAddedEvent<TestComponent1>::EventTypeID, 0);
	ASSERT_NE(ComponentRemovedEvent<TestComponent1>::EventTypeID, 0);

	ASSERT_NE(ComponentAddedEvent<TestComponent2>::EventTypeID, 0);
	ASSERT_NE(ComponentRemovedEvent<TestComponent2>::EventTypeID, 0);

	ASSERT_NE(ComponentAddedEvent<TestComponent1>::EventTypeID, ComponentAddedEvent<TestComponent2>::EventTypeID);
	ASSERT_NE(ComponentRemovedEvent<TestComponent1>::EventTypeID, ComponentRemovedEvent<TestComponent2>::EventTypeID);
	
	ASSERT_NE(EntityCreatedEvent::EventTypeID, 0);
	ASSERT_NE(EntityDestroyedEvent::EventTypeID, 0);

	ASSERT_NE(ComponentAddedEvent<TestComponent1>::EventTypeID, ComponentRemovedEvent<TestComponent1>::EventTypeID);
	ASSERT_NE(EntityCreatedEvent::EventTypeID, EntityDestroyedEvent::EventTypeID);
}

TEST(Events, SendEvents) {

	World::Setup();
	EventManager *eventmanager = World::GetEventManager();

	TestEvent1Listener testListener;

	eventmanager->RegisterListener(&testListener);
	const int numEvents = 1000;
	const int numIterations = 100;

	unsigned long long sum = 0;


	for (int j = 0; j < numIterations; ++j) {
		for (int i = 0; i < numEvents; ++i) {
			sum += i;
			TestEvent1 event;
			event.testInt = i;
			eventmanager->QueueEvent(event);
		}

		eventmanager->DeliverEvents();
	}

	ASSERT_EQ(testListener.sumOfEvents, numEvents*numIterations);
	ASSERT_EQ(testListener.sumOfValues, sum);
}


TEST(Events, CreateEntityEvents) {

	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();

	EntityCreatedEventListener testListener;

	eventmanager->RegisterListener(&testListener);

	const size_t numentities = 10000;

	for (size_t i = 0; i < numentities; ++i) {
		entitymanager->CreateEntity();
	}

	entitymanager->CreateEntities(numentities);

	eventmanager->DeliverEvents();

	ASSERT_EQ(testListener.sumOfEntities, numentities * 2);

	ASSERT_EQ(testListener.entities.size(), numentities * 2);

	for (const Entity &e : testListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}
}

TEST(Events, DestroyEntityEvent) {

	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();

	EntityDestroyedEventListener testListener;

	eventmanager->RegisterListener(&testListener);

	const size_t numentities = 10000;

	std::vector<Entity> evec;

	for (size_t i = 0; i < numentities; ++i) {
		evec.push_back(entitymanager->CreateEntity());
	}

	for (Entity e : evec) {
		entitymanager->DestroyEntity(e);
	}

	EntityArray eArr = entitymanager->CreateEntities(numentities);

	entitymanager->DestroyEntities(eArr);

	eventmanager->DeliverEvents();

	ASSERT_EQ(testListener.sumOfEntities, numentities * 2);

	ASSERT_EQ(testListener.entities.size(), numentities * 2);

	for (const Entity &e : testListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}
}

TEST(Events, ComponentAddedEvents) {

	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	ComponentEventSpawner::instance().RegisterEventSpawnerForComponent<TestComponent1>();

	ComponentAddedEventListener testListener;

	eventmanager->RegisterListener(&testListener);

	const size_t numentities = 10000;

	EntityArray ents = entitymanager->CreateEntities(numentities);

	for (Entity e : ents) {
		componentmanager->AddComponent<TestComponent1>(e);
	}

	eventmanager->DeliverEvents();

	
	ASSERT_EQ(testListener.sumOfEvents, numentities);
	ASSERT_EQ(testListener.entities.size(), numentities);

	for (Entity e : testListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}
}


TEST(Events, EntityArchetypeCreateComponentAdded) {
	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	ComponentEventSpawner::instance().RegisterEventSpawnerForComponent<TestComponent1>();


	EntityArchetype archetype = EntityArchetype::Create<TestComponent1, TestComponent2>();

	ComponentAddedEventListener testListener;

	eventmanager->RegisterListener(&testListener);

	const size_t numentities = 10000;

	EntityArray ents1 = entitymanager->CreateEntities(numentities, archetype);

	eventmanager->DeliverEvents();


	ASSERT_EQ(testListener.sumOfEvents, numentities);
	ASSERT_EQ(testListener.entities.size(), numentities);

	for (Entity e : testListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}
}



TEST(Events, ComponentRemovedEvents) {

	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	ComponentEventSpawner::instance().RegisterEventSpawnerForComponent<TestComponent1>();

	ComponentRemovedEventListener testListener;

	EntityArchetype archetype = EntityArchetype::Create<TestComponent1, TestComponent2>();


	eventmanager->RegisterListener(&testListener);

	const size_t numentities = 10000;

	EntityArray ents = entitymanager->CreateEntities(numentities, archetype);

	for (Entity e : ents) {
		componentmanager->RemoveComponent<TestComponent1>(e);
	}

	eventmanager->DeliverEvents();


	ASSERT_EQ(testListener.sumOfEvents, numentities);
	ASSERT_EQ(testListener.entities.size(), numentities);

	for (Entity e : testListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}
}

TEST(Events, ComponentMoveAddedEvent) {
	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	ComponentEventSpawner::instance().RegisterEventSpawnerForComponent<TestComponent1>();


	EntityArchetype archetype2 = EntityArchetype::Create<TestComponent2>();
	EntityArchetype archetype12 = EntityArchetype::Create<TestComponent1, TestComponent2>();

	ComponentAddedEventListener testListener;

	const size_t numentities = 10000;

	EntityArray ents1 = entitymanager->CreateEntities(numentities, archetype2);

	eventmanager->RegisterListener(&testListener);

	//register after first events have been delivered

	for (Entity e : ents1) {
		componentmanager->MoveToArchetype(e, archetype12);
	}

	eventmanager->DeliverEvents();


	ASSERT_EQ(testListener.sumOfEvents, numentities);
	ASSERT_EQ(testListener.entities.size(), numentities);

	for (Entity e : testListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}
}

TEST(Events, ComponentMoveRemovedEvent) {
	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	ComponentEventSpawner::instance().RegisterEventSpawnerForComponent<TestComponent1>();


	EntityArchetype archetype2 = EntityArchetype::Create<TestComponent2>();
	EntityArchetype archetype12 = EntityArchetype::Create<TestComponent1, TestComponent2>();

	ComponentRemovedEventListener testListener;

	const size_t numentities = 10000;

	EntityArray ents1 = entitymanager->CreateEntities(numentities, archetype12);

	eventmanager->RegisterListener(&testListener);

	//register after first events have been delivered

	for (Entity e : ents1) {
		componentmanager->MoveToArchetype(e, archetype2);
	}

	eventmanager->DeliverEvents();


	ASSERT_EQ(testListener.sumOfEvents, numentities);
	ASSERT_EQ(testListener.entities.size(), numentities);

	for (Entity e : testListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}
}


TEST(Events, EntityDestoyComponentRemovedEvent) {
	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	ComponentEventSpawner::instance().RegisterEventSpawnerForComponent<TestComponent1>();


	EntityArchetype archetype = EntityArchetype::Create<TestComponent1, TestComponent2>();

	ComponentRemovedEventListener testListener;

	const size_t numentities = 10000;

	EntityArray ents1 = entitymanager->CreateEntities(numentities, archetype);

	eventmanager->RegisterListener(&testListener);
	//register after first events have been delivered

	for (Entity e : ents1) {
		entitymanager->DestroyEntity(e);
	}

	eventmanager->DeliverEvents();


	ASSERT_EQ(testListener.sumOfEvents, numentities);
	ASSERT_EQ(testListener.entities.size(), numentities);

	for (Entity e : testListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}
}

TEST(Events, EntityArrayDestoyComponentRemovedEvent) {
	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	ComponentEventSpawner::instance().RegisterEventSpawnerForComponent<TestComponent1>();


	EntityArchetype archetype = EntityArchetype::Create<TestComponent1>();

	ComponentRemovedEventListener testListener;

	const size_t numentities = 10000;

	EntityArray ents1 = entitymanager->CreateEntities(numentities, archetype);

	eventmanager->RegisterListener(&testListener);


	entitymanager->DestroyEntities(ents1);

	eventmanager->DeliverEvents();


	ASSERT_EQ(testListener.sumOfEvents, numentities);
	ASSERT_EQ(testListener.entities.size(), numentities);

	for (Entity e : testListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}
}


TEST(Events, SharedComponentAddedEvents) {

	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	ComponentEventSpawner::instance().RegisterEventSpawnerForSharedComponent<TestSharedComponent1>();

	SharedComponentAddedEventListener testListener;

	TestSharedComponent1 *shared1 = componentmanager->CreateSharedComponent<TestSharedComponent1>();

	eventmanager->RegisterListener(&testListener);

	const size_t numentities = 10000;

	EntityArray ents = entitymanager->CreateEntities(numentities);

	for (Entity e : ents) {
		componentmanager->AddSharedComponent(e, shared1);
	}

	eventmanager->DeliverEvents();


	ASSERT_EQ(testListener.sumOfEvents, numentities);
	ASSERT_EQ(testListener.entities.size(), numentities);
	ASSERT_EQ(testListener.components.size(), numentities);

	for (Entity e : testListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}

	for (TestSharedComponent1 *c : testListener.components) {
		ASSERT_EQ(c, shared1);
	}
}

TEST(Events, SharedComponentRemovedEvents) {

	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	ComponentEventSpawner::instance().RegisterEventSpawnerForSharedComponent<TestSharedComponent1>();

	SharedComponentRemovedEventListener testListener;

	TestSharedComponent1 *shared1 = componentmanager->CreateSharedComponent<TestSharedComponent1>();

	eventmanager->RegisterListener(&testListener);

	const size_t numentities = 10000;

	EntityArray ents = entitymanager->CreateEntities(numentities);

	for (Entity e : ents) {
		componentmanager->AddSharedComponent(e, shared1);
	}

	for (Entity e : ents) {
		componentmanager->RemoveSharedComponent<TestSharedComponent1>(e);
	}

	eventmanager->DeliverEvents();


	ASSERT_EQ(testListener.sumOfEvents, numentities);
	ASSERT_EQ(testListener.entities.size(), numentities);
	ASSERT_EQ(testListener.components.size(), numentities);

	for (Entity e : testListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}

	for (TestSharedComponent1 *c : testListener.components) {
		ASSERT_EQ(c, shared1);
	}
}


TEST(Events, SharedComponentEntityArchetypeCreateDestroy) {

	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	ComponentEventSpawner::instance().RegisterEventSpawnerForSharedComponent<TestSharedComponent1>();

	SharedComponentAddedEventListener testAddedListener;
	SharedComponentRemovedEventListener testRemovedListener;

	TestSharedComponent1 *shared1 = componentmanager->CreateSharedComponent<TestSharedComponent1>();


	EntityArchetype archetype = EntityArchetype::Create<TestComponent1>(shared1);


	eventmanager->RegisterListener(&testAddedListener);
	eventmanager->RegisterListener(&testRemovedListener);


	const size_t numentities = 10000;

	EntityArray ents = entitymanager->CreateEntities(numentities, archetype);

	entitymanager->DestroyEntities(ents);

	eventmanager->DeliverEvents();


	ASSERT_EQ(testAddedListener.sumOfEvents, numentities);
	ASSERT_EQ(testAddedListener.entities.size(), numentities);
	ASSERT_EQ(testAddedListener.components.size(), numentities);

	for (Entity e : testAddedListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}

	for (TestSharedComponent1 *c : testAddedListener.components) {
		ASSERT_EQ(c, shared1);
	}

	ASSERT_EQ(testRemovedListener.sumOfEvents, numentities);
	ASSERT_EQ(testRemovedListener.entities.size(), numentities);
	ASSERT_EQ(testRemovedListener.components.size(), numentities);

	for (Entity e : testRemovedListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}

	for (TestSharedComponent1 *c : testRemovedListener.components) {
		ASSERT_EQ(c, shared1);
	}
}


TEST(Events, SharedComponentMove) {

	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	ComponentEventSpawner::instance().RegisterEventSpawnerForSharedComponent<TestSharedComponent1>();

	SharedComponentAddedEventListener testAddedListener;
	SharedComponentRemovedEventListener testRemovedListener;

	TestSharedComponent1 *shared1 = componentmanager->CreateSharedComponent<TestSharedComponent1>();


	EntityArchetype archetype1 = EntityArchetype::Create<TestComponent1>();
	EntityArchetype archetype2 = EntityArchetype::Create<TestComponent1>(shared1);
	EntityArchetype archetype0 = EntityArchetype();


	eventmanager->RegisterListener(&testAddedListener);
	eventmanager->RegisterListener(&testRemovedListener);


	const size_t numentities = 10000;

	EntityArray ents = entitymanager->CreateEntities(numentities, archetype1);

	for (Entity e : ents) {
		componentmanager->MoveToArchetype(e, archetype2);
	}

	for (Entity e : ents) {
		componentmanager->MoveToArchetype(e, archetype0);
	}

	eventmanager->DeliverEvents();


	ASSERT_EQ(testAddedListener.sumOfEvents, numentities);
	ASSERT_EQ(testAddedListener.entities.size(), numentities);
	ASSERT_EQ(testAddedListener.components.size(), numentities);

	for (Entity e : testAddedListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}

	for (TestSharedComponent1 *c : testAddedListener.components) {
		ASSERT_EQ(c, shared1);
	}

	ASSERT_EQ(testRemovedListener.sumOfEvents, numentities);
	ASSERT_EQ(testRemovedListener.entities.size(), numentities);
	ASSERT_EQ(testRemovedListener.components.size(), numentities);

	for (Entity e : testRemovedListener.entities) {
		ASSERT_NE(e.ID, ENTITY_NULL_ID);
	}

	for (TestSharedComponent1 *c : testRemovedListener.components) {
		ASSERT_EQ(c, shared1);
	}
}
