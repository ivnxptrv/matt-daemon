#include "eventloop.hpp"
#include "listener.hpp"
#include <iostream>
#include <unistd.h>

auto main() -> int {

    Eventloop el;
    Listener l(4242);

    el.addEventSource(l);
    el.run();
    return 0;
}
