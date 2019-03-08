#pragma once
#include "eventlistener.h"

#ifndef ECS_NO_TSL
#include "../tsl/robin_map.h"
#endif

namespace gleng {

	class IEventQueue {
	public:
		virtual void DeliverEvents() = 0;
	};

	template <class T>
	class EventQueue : public IEventQueue {
		std::vector<IEventListener<T>*> listeners;
		std::vector<T> events;
	public:
		virtual void DeliverEvents() {
			if (events.size() == 0) {
				return;
			} else {
				EventIterator<T> eit(&events[0], events.size());
				for (IEventListener<T>* listener : listeners) {
					listener->ProcessEvents(eit);
				}

				events.clear();
			}
		}

		inline void AddListener(IEventListener<T>* listener) {
			auto found = std::find(listeners.begin(), listeners.end(), listener);
			if (found == listeners.end()) {
				listeners.push_back(listener);
			}
		}

		inline void RemoveListener(IEventListener<T>* listener) {
			auto found = std::find(listeners.begin(), listeners.end(), listener);
			if (found != listeners.end()) {
				listeners.erase(found);
			}
		}

		inline void AddEvent(const T& e) {
			events.push_back(e);
		}
	};

	class EventManager {
#ifdef ECS_NO_TSL
		std::unordered_map<type_hash, IEventQueue*, util::typehasher> _eventQueues;
#else
		tsl::robin_map<type_hash, IEventQueue*, util::typehasher> _eventQueues;
#endif // ECS_NO_TSL


		template<class T>
		inline EventQueue<T>* GetOrCreateEventQueue() {
			CHECK_T_IS_EVENT;
			type_hash type = IEvent<T>::EventTypeID;
			auto found = _eventQueues.find(type);
			if (found != _eventQueues.end()) {
				return static_cast<EventQueue<T>*>(found->second);
			} else {
				EventQueue<T> *newQue = new EventQueue<T>();
				_eventQueues.emplace(type, newQue);
				return newQue;
			}
		}
	public:

		inline void DeliverEvents() {
			for (auto keyval : _eventQueues) {
				keyval.second->DeliverEvents();
			}
		}

		template <class T>
		inline void RegisterListener(IEventListener<T>* listener) {
			CHECK_T_IS_EVENT;
			EventQueue<T>* queue = GetOrCreateEventQueue<T>();
			queue->AddListener(listener);
		}

		template <class T>
		inline void UnRegisterListener(IEventListener<T>* listener) {
			CHECK_T_IS_EVENT;
			EventQueue<T>* queue = GetOrCreateEventQueue<T>();
			queue->RemoveListener(listener);
		}

		template <class T>
		inline void QueueEvent(const T& e) {
			CHECK_T_IS_EVENT;
			EventQueue<T>* queue = GetOrCreateEventQueue<T>();
			queue->AddEvent(e);
		}

		inline void Clear() {
			for (auto keyval : _eventQueues) {
				delete(keyval.second);
			}
			_eventQueues.clear();
		}

		inline ~EventManager() {
			Clear();
		}
	};

}