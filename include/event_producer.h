#ifndef EVENT_PRODUCER_H
#define EVENT_PRODUCER_H

#include <string>
#include <Poco/JSON/Object.h>

class EventProducer {
public:
    static bool initialize();
    static void shutdown();
    
    static void publishBookingCreated(int bookingId, int userId, int hotelId, const std::string& startDate, const std::string& endDate);
    
    static void publishBookingCancelled(int bookingId, int userId, int hotelId);
    
private:
    static std::string generateEventId();
    static std::string getCurrentTimestamp();
    static bool sendToRabbitMQ(const std::string& exchange, const std::string& routingKey, const Poco::JSON::Object::Ptr& event);
};

#endif
