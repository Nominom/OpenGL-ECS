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

class ComponentAddedEventListener : public IEventListener<ComponentAddedEvent> {
public:
	std::vector<Entity> entities;
	std::vector<type_hash> types;
	int sumOfEvents = 0;

	virtual void ProcessEvents(const EventIterator<ComponentAddedEvent> &eventIterator) override {
		for (const ComponentAddedEvent &event : eventIterator) {
			++sumOfEvents;
			entities.push_back(event.entity);
			types.push_back(event.componentType);
		}
	}
};

class ComponentRemovedEventListener : public IEventListener<ComponentRemovedEvent> {
public:
	std::vector<Entity> entities;
	std::vector<type_hash> types;
	int sumOfEvents = 0;

	virtual void ProcessEvents(const EventIterator<ComponentRemovedEvent> &eventIterator) override{
		for (const ComponentRemovedEvent &event : eventIterator) {
			++sumOfEvents;
			entities.push_back(event.entity);
			types.push_back(event.componentType);
		}
	}
};

TEST(Events, TypeID) {

	ASSERT_NE(ComponentAddedEvent::EventTypeID, 0);
	ASSERT_NE(ComponentRemovedEvent::EventTypeID, 0);
	ASSERT_NE(EntityCreatedEvent::EventTypeID, 0);
	ASSERT_NE(EntityDestroyedEvent::EventTypeID, 0);

	ASSERT_NE(ComponentAddedEvent::EventTypeID, ComponentRemovedEvent::EventTypeID);
	ASSERT_NE(EntityCreatedEvent::EventTypeID, EntityDestroyedEvent::EventTypeID);
}

TEST(Events, SendEvents) {

	World::Setup();
	EventManager *eventmanager = World::GetEventManager();

	TestEvent1Listener testListener;

	eventmanager->RegisterListener(&testListener);
	const int numEvents = 1000;
	const int numIterations = 1000;

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

	const size_t numentities = 1000;

	for (size_t i = 0; i < numentities; ++i) {
		entitymanager->CreateEntity();
	}

	entitymanager->CreateEntitites(numentities);

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

	ComponentAddedEventListener testListener;

	eventmanager->RegisterListener(&testListener);

	const size_t numentities = 1000;

	EntityArray ents = entitymanager->CreateEntitites(numentities);

	for (Entity e : ents) {
		componentmanager->AddComponent<TestComponent2>(e);
	}

	eventmanager->DeliverEvents();

	
	ASSERT_EQ(testListener.sumOfEvents, numentities);
	ASSERT_EQ(testListener.types.size(), numentities);
	ASSERT_EQ(testListener.entities.size(), numentities);

	for (type_hash type : testListener.types) {
		ASSERT_EQ(type, TestComponent2::ComponentTypeID);
	}
}


TEST(Events, EntityArchetypeCreateComponentAdded) {
	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();


	EntityArchetype archetype = EntityArchetype::Create<TestComponent2>();

	ComponentAddedEventListener testListener;

	eventmanager->RegisterListener(&testListener);

	const size_t numentities = 1000;

	EntityArray ents1 = entitymanager->CreateEntitites(numentities, archetype);

	eventmanager->DeliverEvents();


	ASSERT_EQ(testListener.sumOfEvents, numentities);
	ASSERT_EQ(testListener.types.size(), numentities);
	ASSERT_EQ(testListener.entities.size(), numentities);

	for (type_hash type : testListener.types) {
		ASSERT_EQ(type, TestComponent2::ComponentTypeID);
	}
}



TEST(Events, ComponentRemovedEvents) {

	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();

	ComponentRemovedEventListener testListener;

	EntityArchetype archetype = EntityArchetype::Create<TestComponent2>();


	eventmanager->RegisterListener(&testListener);

	const size_t numentities = 1000;

	EntityArray ents = entitymanager->CreateEntitites(numentities, archetype);

	for (Entity e : ents) {
		componentmanager->RemoveComponent<TestComponent2>(e);
	}

	eventmanager->DeliverEvents();


	ASSERT_EQ(testListener.sumOfEvents, numentities);
	ASSERT_EQ(testListener.types.size(), numentities);
	ASSERT_EQ(testListener.entities.size(), numentities);

	for (type_hash type : testListener.types) {
		ASSERT_EQ(type, TestComponent2::ComponentTypeID);
	}
}

TEST(Events, ComponentMoveAddedEvent) {
	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();


	EntityArchetype archetype2 = EntityArchetype::Create<TestComponent2>();
	EntityArchetype archetype12 = EntityArchetype::Create<TestComponent1, TestComponent2>();

	ComponentAddedEventListener testListener;

	const size_t numentities = 1000;

	EntityArray ents1 = entitymanager->CreateEntitites(numentities, archetype2);

	eventmanager->DeliverEvents();

	//register after first events have been delivered
	eventmanager->RegisterListener(&testListener);

	for (Entity e : ents1) {
		componentmanager->MoveToArchetype(e, archetype12);
	}

	eventmanager->DeliverEvents();


	ASSERT_EQ(testListener.sumOfEvents, numentities);
	ASSERT_EQ(testListener.types.size(), numentities);
	ASSERT_EQ(testListener.entities.size(), numentities);

	for (type_hash type : testListener.types) {
		ASSERT_EQ(type, TestComponent1::ComponentTypeID);
	}
}

TEST(Events, ComponentMoveRemovedEvent) {
	World::Setup();
	EventManager *eventmanager = World::GetEventManager();
	EntityManager *entitymanager = World::GetEntityManager();
	ComponentManager *componentmanager = World::GetComponentManager();


	EntityArchetype archetype2 = EntityArchetype::Create<TestComponent2>();
	EntityArchetype archetype12 = EntityArchetype::Create<TestComponent1, TestComponent2>();

	ComponentRemovedEventListener testListener;

	const size_t numentities = 1000;

	EntityArray ents1 = entitymanager->CreateEntitites(numentities, archetype12);

	eventmanager->DeliverEvents();

	//register after first events have been delivered
	eventmanager->RegisterListener(&testListener);

	for (Entity e : ents1) {
		componentmanager->MoveToArchetype(e, archetype2);
	}

	eventmanager->DeliverEvents();


	ASSERT_EQ(testListener.sumOfEvents, numentities);
	ASSERT_EQ(testListener.types.size(), numentities);
	ASSERT_EQ(testListener.entities.size(), numentities);

	for (type_hash type : testListener.types) {
		ASSERT_EQ(type, TestComponent1::ComponentTypeID);
	}
}