# EasyEvents

Simple Header-Only C++14 (and newer) event handling library

## Requirements

- C++20 compatible compiler
- thats it :D

## Usage

Include the header:

```cpp
#include "EasyEvents.hpp"
```

Create a struct to hold your event(s):

```cpp
struct EventA {
    int value;
};

struct EventB {
    std::string message;
};
```

Listen to / Deafen events by struct:

```cpp
EasyEvents::EventDispatcher dispatcher;

auto tokenA = dispatcher.listen<EventA>([](const EventA& e) {
    std::cout << "[EventA] Received value: " << e.value << "\n";
});
dispatcher.trigger(EventA{ 84 });
dispatcher.deafen(tokenA);
```

*(optional)* Add priorities to listeners:
> `EasyEvents::Priority` can be edited if you need a more specific order! (Lower values are called first)

```cpp
auto tokenA1 = dispatcher.listen<EventA>([](const EventA& e) {
    std::cout << "[EventA - FIRST] Received value: " << e.value << "\n";
}, EasyEvents::Priority::FIRST);

auto tokenA2 = dispatcher.listen<EventA>([](const EventA& e) {
    std::cout << "[EventA - LAST] Received value: " << e.value << "\n";
}, EasyEvents::Priority::LAST);
```
