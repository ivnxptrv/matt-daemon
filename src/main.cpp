#include "eventloop.hpp"
#include "listener.hpp"
#include <iostream>
#include <unistd.h>

auto main() -> int {

    Eventloop el;

    el.addEventSource(new Listener(4242));
    el.run();
    return 0;
}
