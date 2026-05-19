#include "event_producer.h"

#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/JSON/Stringifier.h>

#include <SimpleAmqpClient/SimpleAmqpClient.h>

#include <iostream>
#include <sstream>

static AmqpClient::Channel::ptr_t rabbitChannel = nullptr;


// Генерация event id
std::string EventProducer::generateEventId() {
    Poco::UUIDGenerator generator;
    Poco::UUID uuid = generator.createRandom();

    return uuid.toString();
}

std::string EventProducer::getCurrentTimestamp() {
    Poco::DateTime currentTime;

    return Poco::DateTimeFormatter::format(
        currentTime,
        "%Y-%m-%dT%H:%M:%SZ"
    );
}

bool EventProducer::initialize() {
    try {
        rabbitChannel = AmqpClient::Channel::Create(
            "rabbitmq",
            5672,
            "guest",
            "guest",
            "/"
        );

        // Exchange для событий бронирования
        rabbitChannel->DeclareExchange(
            "booking-events",
            AmqpClient::Channel::EXCHANGE_TYPE_DIRECT,
            false,
            true
        );

        std::cout << "[EventProducer] RabbitMQ connected" << std::endl;

        return true;
    }
    catch (const std::exception& ex) {
        std::cerr << "[EventProducer] init error: " << ex.what() << std::endl;
        return false;
    }
}

void EventProducer::shutdown() {
    if (!rabbitChannel) {
        return;
    }

    try {
        rabbitChannel.reset();

        std::cout << "[EventProducer] stopped" << std::endl;
    }
    catch (const std::exception& ex) {
        std::cerr  << "[EventProducer] shutdown failed: " << ex.what()<< std::endl;
    }
}

bool EventProducer::sendToRabbitMQ(const std::string& exchange, const std::string& routingKey, const Poco::JSON::Object::Ptr& eventData) {
    if (!rabbitChannel) {
        std::cerr << "RabbitMQ channel is not initialized" << std::endl;
        return false;
    }

    try {
        std::stringstream jsonStream;

        Poco::JSON::Stringifier::stringify(
            eventData,
            jsonStream
        );

        std::string payload = jsonStream.str();

        auto message = AmqpClient::BasicMessage::Create(payload);

        rabbitChannel->BasicPublish(
            exchange,
            routingKey,
            message
        );

        std::cout << "Published event -> " << routingKey<< std::endl;

        return true;
    }
    catch (const std::exception& ex) {
        std::cerr << "RabbitMQ publish error: "  << ex.what() << std::endl;
        return false;
    }
}

// Создание бронирования
void EventProducer::publishBookingCreated(int bookingId, int userId, int hotelId, const std::string& startDate, const std::string& endDate) {
    Poco::JSON::Object::Ptr event = new Poco::JSON::Object();

    event->set("event_type", "booking.created");
    event->set("event_id", generateEventId());
    event->set("timestamp", getCurrentTimestamp());

    Poco::JSON::Object::Ptr payload = new Poco::JSON::Object();

    payload->set("booking_id", bookingId);
    payload->set("user_id", userId);
    payload->set("hotel_id", hotelId);

    payload->set("start_date", startDate);
    payload->set("end_date", endDate);

    event->set("data", payload);

    sendToRabbitMQ(
        "booking-events",
        "booking.created",
        event
    );
}

// Отмена бронирования
void EventProducer::publishBookingCancelled(int bookingId, int userId, int hotelId) {
    Poco::JSON::Object::Ptr event = new Poco::JSON::Object();

    event->set("event_type", "booking.cancelled");
    event->set("event_id", generateEventId());
    event->set("timestamp", getCurrentTimestamp());

    Poco::JSON::Object::Ptr payload = new Poco::JSON::Object();

    payload->set("booking_id", bookingId);
    payload->set("user_id", userId);
    payload->set("hotel_id", hotelId);

    event->set("data", payload);

    sendToRabbitMQ(
        "booking-events",
        "booking.cancelled",
        event
    );
}