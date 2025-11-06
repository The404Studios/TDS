#pragma once

#include <functional>
#include <map>
#include <vector>
#include <memory>
#include <string>
#include <any>

// Event data - can hold any type of data
class EventData {
public:
    EventData() = default;

    template<typename T>
    void set(const std::string& key, const T& value) {
        data[key] = value;
    }

    template<typename T>
    T get(const std::string& key, const T& defaultValue = T()) const {
        auto it = data.find(key);
        if (it != data.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    bool has(const std::string& key) const {
        return data.find(key) != data.end();
    }

    void clear() {
        data.clear();
    }

private:
    std::map<std::string, std::any> data;
};

// Event listener - callback function
using EventListener = std::function<void(const EventData&)>;

// Event subscription handle
class EventSubscription {
public:
    EventSubscription(size_t id, const std::string& event)
        : id(id), eventName(event) {}

    size_t getId() const { return id; }
    const std::string& getEventName() const { return eventName; }

private:
    size_t id;
    std::string eventName;
};

// Event bus - centralized event system
class EventBus {
public:
    static EventBus& getInstance();

    // Subscribe to an event
    std::shared_ptr<EventSubscription> subscribe(const std::string& eventName, EventListener listener);

    // Unsubscribe from an event
    void unsubscribe(std::shared_ptr<EventSubscription> subscription);
    void unsubscribe(size_t subscriptionId);

    // Emit an event immediately
    void emit(const std::string& eventName, const EventData& data = EventData());

    // Queue an event to be processed later
    void queue(const std::string& eventName, const EventData& data = EventData());

    // Process all queued events
    void processQueue();

    // Clear all subscriptions
    void clear();

    // Get statistics
    size_t getSubscriberCount(const std::string& eventName) const;
    size_t getTotalSubscribers() const;
    size_t getQueuedEventCount() const;

private:
    EventBus() : nextId(1) {}

    struct ListenerData {
        size_t id;
        EventListener listener;
    };

    struct QueuedEvent {
        std::string eventName;
        EventData data;
    };

    std::map<std::string, std::vector<ListenerData>> listeners;
    std::vector<QueuedEvent> eventQueue;
    size_t nextId;
};

// Common event names (you can add more as needed)
namespace Events {
    // UI Events
    constexpr const char* UI_BUTTON_CLICKED = "ui.button.clicked";
    constexpr const char* UI_TEXT_CHANGED = "ui.text.changed";
    constexpr const char* UI_FOCUS_GAINED = "ui.focus.gained";
    constexpr const char* UI_FOCUS_LOST = "ui.focus.lost";
    constexpr const char* UI_HOVER_ENTER = "ui.hover.enter";
    constexpr const char* UI_HOVER_EXIT = "ui.hover.exit";

    // Scene Events
    constexpr const char* SCENE_LOADED = "scene.loaded";
    constexpr const char* SCENE_UNLOADED = "scene.unloaded";
    constexpr const char* SCENE_TRANSITION_START = "scene.transition.start";
    constexpr const char* SCENE_TRANSITION_END = "scene.transition.end";

    // Network Events
    constexpr const char* NETWORK_CONNECTED = "network.connected";
    constexpr const char* NETWORK_DISCONNECTED = "network.disconnected";
    constexpr const char* NETWORK_ERROR = "network.error";
    constexpr const char* NETWORK_PACKET_RECEIVED = "network.packet.received";

    // Auth Events
    constexpr const char* AUTH_LOGIN_SUCCESS = "auth.login.success";
    constexpr const char* AUTH_LOGIN_FAILED = "auth.login.failed";
    constexpr const char* AUTH_LOGOUT = "auth.logout";
    constexpr const char* AUTH_REGISTER_SUCCESS = "auth.register.success";
    constexpr const char* AUTH_REGISTER_FAILED = "auth.register.failed";

    // Game Events
    constexpr const char* GAME_LOBBY_JOINED = "game.lobby.joined";
    constexpr const char* GAME_LOBBY_LEFT = "game.lobby.left";
    constexpr const char* GAME_MATCH_STARTED = "game.match.started";
    constexpr const char* GAME_MATCH_ENDED = "game.match.ended";
    constexpr const char* GAME_PLAYER_JOINED = "game.player.joined";
    constexpr const char* GAME_PLAYER_LEFT = "game.player.left";

    // Inventory Events
    constexpr const char* INVENTORY_ITEM_ADDED = "inventory.item.added";
    constexpr const char* INVENTORY_ITEM_REMOVED = "inventory.item.removed";
    constexpr const char* INVENTORY_ITEM_MOVED = "inventory.item.moved";
    constexpr const char* INVENTORY_UPDATED = "inventory.updated";

    // Merchant Events
    constexpr const char* MERCHANT_ITEM_BOUGHT = "merchant.item.bought";
    constexpr const char* MERCHANT_ITEM_SOLD = "merchant.item.sold";
    constexpr const char* MERCHANT_OPENED = "merchant.opened";
    constexpr const char* MERCHANT_CLOSED = "merchant.closed";
}

// Helper class for scoped subscriptions (auto-unsubscribe on destruction)
class ScopedEventSubscription {
public:
    ScopedEventSubscription(std::shared_ptr<EventSubscription> sub)
        : subscription(sub) {}

    ~ScopedEventSubscription() {
        if (subscription) {
            EventBus::getInstance().unsubscribe(subscription);
        }
    }

    // No copy
    ScopedEventSubscription(const ScopedEventSubscription&) = delete;
    ScopedEventSubscription& operator=(const ScopedEventSubscription&) = delete;

    // Allow move
    ScopedEventSubscription(ScopedEventSubscription&& other) noexcept
        : subscription(std::move(other.subscription)) {}

    ScopedEventSubscription& operator=(ScopedEventSubscription&& other) noexcept {
        if (this != &other) {
            subscription = std::move(other.subscription);
        }
        return *this;
    }

private:
    std::shared_ptr<EventSubscription> subscription;
};
