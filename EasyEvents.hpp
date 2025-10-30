#pragma once
#include <algorithm>
#include <concepts>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <print>
#include <ranges>
#include <shared_mutex>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#define _EVT_LOG(msg) std::println("[EasyEvents] {}", msg)

namespace EasyEvents {

    template <typename T>
    concept Event = std::is_class_v<T> && std::is_default_constructible_v<T>;

    class AbstractDispatcher {
    public:
        virtual ~AbstractDispatcher() = default;
    };

    template <Event E>
    class Dispatcher final : public AbstractDispatcher {
    public:
        using Callback = std::function<void(const E&, std::shared_ptr<E>)>;

        struct Listener {
            Callback callback;
            int priority;
            bool once;
        };

        void subscribe(Callback&& cb, int priority, bool once) {
            std::unique_lock lock(mutex_);
            listeners_.push_back({ std::move(cb), priority, once });
            std::ranges::sort(listeners_, std::ranges::greater{}, &Listener::priority);
        }

        void unsubscribe_all() {
            std::unique_lock lock(mutex_);
            listeners_.clear();
        }

        void dispatch(const E& evt) {
            std::vector<Listener> snapshot;
            {
                std::shared_lock lock(mutex_);
                snapshot = listeners_;
            }

            auto shared_evt = std::make_shared<E>(evt);
            for (auto it = snapshot.begin(); it != snapshot.end(); ++it) {
                try {
                    it->callback(evt, shared_evt);
                }
                catch (const std::exception& ex) {
                    _EVT_LOG(std::format("Exception in listener: {}", ex.what()));
                }
                if (it->once) {
                    unsubscribe_listener(it->callback);
                }
            }
        }

    private:
        void unsubscribe_listener(const Callback& target) {
            std::unique_lock lock(mutex_);
            listeners_.erase(
                std::remove_if(listeners_.begin(), listeners_.end(),
                    [&](const Listener& l) {
                        // Can't compare std::function directly; just clear once
                        return l.once;
                    }),
                listeners_.end());
        }

        mutable std::shared_mutex mutex_;
        std::vector<Listener> listeners_;
    };

    class EventBus {
    public:
        EventBus() = default;
        ~EventBus() = default;

        EventBus(const EventBus&) = delete;
        EventBus& operator=(const EventBus&) = delete;

        template <Event E>
        void subscribe(std::function<void(const E&, std::shared_ptr<E>)> cb,
            int priority = 0) {
            get_dispatcher<E>().subscribe(std::move(cb), priority, false);
        }

        template <Event E>
        void subscribe_once(std::function<void(const E&, std::shared_ptr<E>)> cb,
            int priority = 0) {
            get_dispatcher<E>().subscribe(std::move(cb), priority, true);
        }

        template <Event E>
        void unsubscribe_all() {
            get_dispatcher<E>().unsubscribe_all();
        }

        template <Event E>
        void emit(const E& evt) {
            get_dispatcher<E>().dispatch(evt);
        }

        void clear() {
            std::unique_lock lock(mutex_);
            dispatchers_.clear();
        }

    private:
        template <Event E>
        Dispatcher<E>& get_dispatcher() {
            const std::type_index type = typeid(E);
            std::unique_lock lock(mutex_);
            auto it = dispatchers_.find(type);
            if (it == dispatchers_.end()) {
                auto disp = std::make_shared<Dispatcher<E>>();
                dispatchers_[type] = disp;
                return *disp;
            }
            return *static_cast<Dispatcher<E>*>(dispatchers_[type].get());
        }

        mutable std::shared_mutex mutex_;
        std::unordered_map<std::type_index, std::shared_ptr<AbstractDispatcher>>
            dispatchers_;
    };

}  // namespace EasyEvents
