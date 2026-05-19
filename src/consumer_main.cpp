#include "event_consumer.h"
#include <iostream>
#include <csignal>

void signalHandler(int signum) {
    std::cout << "\n\n👋 Event Consumer shutdown\n" << std::endl;
    exit(signum);
}

int main(int argc, char** argv) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    EventConsumer::run();
    return 0;
}
