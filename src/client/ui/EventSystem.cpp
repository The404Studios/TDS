#include "EventSystem.h"
#include <algorithm>

EventBus& EventBus::getInstance() {
    static EventBus instance;
    return instance;
}

std::shared_ptr<EventSubscription> EventBus::subscribe(const std::string& eventName, EventListener listener) {
    size_t id = nextId++;
    listeners[eventName].push_back({ id, listener });
    return std::make_shared<EventSubscription>(id, eventName);
}

void EventBus::unsubscribe(std::shared_ptr<EventSubscription> subscription) {
    if (!subscription) return;
    unsubscribe(subscription->getId());
}

void EventBus::unsubscribe(size_t subscriptionId) {
    for (auto& pair : listeners) {
        auto& listenerList = pair.second;
        listenerList.erase(
            std::remove_if(listenerList.begin(), listenerList.end(),
                [subscriptionId](const ListenerData& data) {
                    return data.id == subscriptionId;
                }),
            listenerList.end()
        );
    }
}

void EventBus::emit(const std::string& eventName, const EventData& data) {
    auto it = listeners.find(eventName);
    if (it != listeners.end()) {
        // Copy the listeners list to avoid issues if a listener modifies the list
        auto listenersCopy = it->second;
        for (const auto& listenerData : listenersCopy) {
            if (listenerData.listener) {
                listenerData.listener(data);
            }
        }
    }
}

void EventBus::queue(const std::string& eventName, const EventData& data) {
    eventQueue.push_back({ eventName, data });
}

void EventBus::processQueue() {
    // Process all queued events
    std::vector<QueuedEvent> queueCopy = std::move(eventQueue);
    eventQueue.clear();

    for (const auto& queuedEvent : queueCopy) {
        emit(queuedEvent.eventName, queuedEvent.data);
    }
}

void EventBus::clear() {
    listeners.clear();
    eventQueue.clear();
}

size_t EventBus::getSubscriberCount(const std::string& eventName) const {
    auto it = listeners.find(eventName);
    return (it != listeners.end()) ? it->second.size() : 0;
}

size_t EventBus::getTotalSubscribers() const {
    size_t total = 0;
    for (const auto& pair : listeners) {
        total += pair.second.size();
    }
    return total;
}

size_t EventBus::getQueuedEventCount() const {
    return eventQueue.size();
}
