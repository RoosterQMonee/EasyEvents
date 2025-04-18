#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <mutex>
#include <memory>
#include <typeindex>

namespace EasyEvents {
    enum class Priority {
        FIRST = 0,
        DEFAULT = 1,
        LAST = 2
    };

    struct ListenerToken {
        std::type_index type;
        size_t id;
    };

    class ListenerCollectionBase {
    public:
        virtual ~ListenerCollectionBase() = default;
        virtual void removeListenerById(size_t id) = 0;
    };

    template <typename T>
    class ListenerCollection : public ListenerCollectionBase {
    public:
        using CallbackType = std::function<void(const T&)>;

        struct Listener {
            CallbackType callback;
            Priority priority;
            size_t id;
        };

        void addListener(const Listener& listener) {
            listeners.push_back(listener);
            std::sort(listeners.begin(), listeners.end(), [](const Listener& a, const Listener& b) {
                return static_cast<int>(a.priority) < static_cast<int>(b.priority);
            });
        }

        void trigger(const T& event) {
            auto localListeners = listeners;
            for (auto& listener : localListeners) {
                listener.callback(event);
            }
        }

        virtual void removeListenerById(size_t id) override {
            auto it = std::remove_if(listeners.begin(), listeners.end(),
                [id](const Listener& l) { return l.id == id; });
            listeners.erase(it, listeners.end());
        }

    private:
        std::vector<Listener> listeners;
    };

    class EventDispatcher {
    public:
        template <typename T>
        ListenerToken listen(std::function<void(const T&)> callback, Priority priority = Priority::DEFAULT) {
            std::lock_guard<std::mutex> lock(mutex);
            auto type = std::type_index(typeid(T));
            size_t id = nextListenerId++;
            ListenerToken token{ type, id };

            auto it = collections.find(type);
            if (it == collections.end()) {
                auto collection = std::make_unique<ListenerCollection<T>>();
                collection->addListener({ callback, priority, id });
                collections[type] = std::move(collection);
            } else {
                auto collection = static_cast<ListenerCollection<T>*>(it->second.get());
                collection->addListener({ callback, priority, id });
            }
            return token;
        }

        template <typename T>
        void trigger(const T& event) {
            std::lock_guard<std::mutex> lock(mutex);
            auto type = std::type_index(typeid(T));
            auto it = collections.find(type);

            if (it != collections.end()) {
                auto collection = static_cast<ListenerCollection<T>*>(it->second.get());
                collection->trigger(event);
            }
        }

        void deafen(const ListenerToken& token) {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = collections.find(token.type);

            if (it != collections.end()) {
                it->second->removeListenerById(token.id);
            }
        }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<ListenerCollectionBase>> collections;
        size_t nextListenerId = 0;
        std::mutex mutex;
    };
} // namespace event