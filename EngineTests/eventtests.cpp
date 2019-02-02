#include "pch.h"


class TestEvent1Listener : public IEventListener<TestEvent1> {
public:
	int sumOfEvents = 0;
	unsigned long long  sumOfValues = 0;

	void ProcessEvents(const EventIterator<TestEvent1> &eventIterator) {
		for (const TestEvent1 &event : eventIterator) {
			sumOfEvents++;
			sumOfValues += event.testInt;
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
	const int numEvents = 1000000;
	unsigned long long sum = 0;
	for (int i = 0; i < numEvents; i++) {
		sum += i;
		TestEvent1 event;
		event.testInt = i;
		eventmanager->QueueEvent(event);
	}

	eventmanager->DeliverEvents();

	ASSERT_EQ(testListener.sumOfEvents, numEvents);
	ASSERT_EQ(testListener.sumOfValues, sum);
}