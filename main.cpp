#include <iostream>
#include "EasyEvents.hpp"

struct EventA {
    int value;
};

struct EventB {
    std::string message;
};

int main() {
    EasyEvents::EventDispatcher dispatcher;

    auto tokenA1 = dispatcher.listen<EventA>([](const EventA& e) {
        std::cout << "[EventA - FIRST] Received value: " << e.value << "\n";
    }, EasyEvents::Priority::FIRST);

    auto tokenA2 = dispatcher.listen<EventA>([](const EventA& e) {
        std::cout << "[EventA - DEFAULT] Received value: " << e.value << "\n";
    });

    auto tokenA3 = dispatcher.listen<EventA>([](const EventA& e) {
        std::cout << "[EventA - LAST] Received value: " << e.value << "\n";
    }, EasyEvents::Priority::LAST);

    auto tokenB = dispatcher.listen<EventB>([](const EventB& e) {
        std::cout << "[EventB] Received message: " << e.message << "\n";
    });

    std::cout << "Triggering EventA with value 42:" << std::endl;
    dispatcher.trigger(EventA{ 42 });

    std::cout << "\nTriggering EventB with message \"Hello World\": " << std::endl;
    dispatcher.trigger(EventB{ "Hello World" });

    dispatcher.deafen(tokenA2);
    std::cout << "\nAfter removing DEFAULT listener for EventA, triggering with value 84:" << std::endl;
    dispatcher.trigger(EventA{ 84 });

    return 0;
}
