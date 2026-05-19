#ifndef EVENT_CONSUMER_H
#define EVENT_CONSUMER_H

#include <string>

class EventConsumer {
public:
    static void run();
    
private:
    static void handleBookingCreated(const std::string& json);
    static void handleBookingCancelled(const std::string& json);
    static void printEvent(const std::string& eventType, const std::string& details);
};

#endif
