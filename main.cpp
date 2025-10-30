#include "EasyEvents.h"
#include <print>
#include <string>

struct MyEvent {
    std::string event_string_1;
    bool cancelled = false;
};

int main() {
    EasyEvents::EventBus bus;

    bus.subscribe<MyEvent>(
        [](const MyEvent& e, std::shared_ptr<MyEvent> shared) {
            std::println("Listener A got: {}", e.event_string_1);
            if (e.event_string_1 == "cancel_me") {
                shared->cancelled = true;
                std::println("Listener A cancelled event.");
            }
        },
        10);

    bus.subscribe<MyEvent>(
        [](const MyEvent& e, std::shared_ptr<MyEvent> shared) {
            if (shared->cancelled)
                std::println("Listener B: event was cancelled!");
            else
                std::println("Listener B: {}", e.event_string_1);
        },
        5);

    bus.subscribe_once<MyEvent>(
        [](const MyEvent& e, std::shared_ptr<MyEvent>) {
            std::println("One-shot listener triggered once with '{}'",
                e.event_string_1);
        });

    bus.emit(MyEvent{ "hello world" });
    bus.emit(MyEvent{ "cancel_me" });
    bus.emit(MyEvent{ "after cancel" });
}
