#include "event_consumer.h"

#include <SimpleAmqpClient/SimpleAmqpClient.h>

#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/Dynamic/Var.h>

#include <iostream>
#include <sstream>
#include <iomanip>

#include <chrono>
#include <thread>

void EventConsumer::printEvent(const std::string& eventType, const std::string& details) {
    auto now = std::chrono::system_clock::now();

    auto currentTime = std::chrono::system_clock::to_time_t(now);

    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::cout << "\n" << std::string(70, '=') << std::endl;

    std::cout
        << "[" << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << milliseconds.count() << "] "
        << eventType
        << std::endl;

    std::cout << details;

    std::cout << std::string(70, '=') << "\n" << std::endl;
}

void EventConsumer::handleBookingCreated(const std::string& json) {
    try {
        Poco::JSON::Parser jsonParser;

        auto parsedJson = jsonParser.parse(json).extract<Poco::JSON::Object::Ptr>();

        auto data = parsedJson->getObject("data");

        int bookingId = data->getValue<int>("booking_id");
        int userId = data->getValue<int>("user_id");
        int hotelId = data->getValue<int>("hotel_id");

        std::string startDate =
            data->getValue<std::string>("start_date");

        std::string endDate =
            data->getValue<std::string>("end_date");

        std::stringstream detailsStream;

        detailsStream
            << "  Booking ID: " << bookingId << "\n"
            << "  User ID: " << userId << "\n"
            << "  Hotel ID: " << hotelId << "\n"
            << "  Period: " << startDate
            << " -> " << endDate << "\n";

        printEvent(
            "BOOKING CREATED",
            detailsStream.str()
        );

    } catch (const std::exception& ex) {
        std::cerr << "Failed to parse booking.created: "  << ex.what()<< std::endl;
    }
}

void EventConsumer::handleBookingCancelled(const std::string& json) {
    try {
        Poco::JSON::Parser parser;

        auto parsed =parser.parse(json).extract<Poco::JSON::Object::Ptr>();

        auto data = parsed->getObject("data");

        int bookingId = data->getValue<int>("booking_id");
        int userId = data->getValue<int>("user_id");
        int hotelId = data->getValue<int>("hotel_id");

        std::stringstream output;

        output
            << "  Booking ID: " << bookingId << "\n"
            << "  User ID: " << userId << "\n"
            << "  Hotel ID: " << hotelId << "\n";

        printEvent(
            "BOOKING CANCELLED",
            output.str()
        );

    } catch (const std::exception& ex) {
        std::cerr << "Failed to parse booking.cancelled: "<< ex.what()<< std::endl;
    }
}

void EventConsumer::run() {
    try {
        std::cout
            << "\nEvent Consumer started (booking-events listener)\n"
            << std::endl;

        const char* rabbitmqHost = std::getenv("RABBITMQ_HOST");

        std::string host = rabbitmqHost ? rabbitmqHost : "rabbitmq";

        auto channel = AmqpClient::Channel::Create(
            host,
            5672,
            "guest",
            "guest",
            "/"
        );

        channel->DeclareExchange(
            "booking-events",
            AmqpClient::Channel::EXCHANGE_TYPE_DIRECT,
            false,
            true
        );

        std::string queueName = channel->DeclareQueue("", false, true, true);

        channel->BindQueue(
            queueName,
            "booking-events",
            "booking.created"
        );

        channel->BindQueue(
            queueName,
            "booking-events",
            "booking.cancelled"
        );

        std::string consumerTag = channel->BasicConsume(queueName, "", true, false);

        std::cout << "RabbitMQ connected" << std::endl;
        std::cout << "Queue: " << queueName << std::endl;
        std::cout << "Consumer tag: " << consumerTag << std::endl;

        while (true) {
            try {
                AmqpClient::Envelope::ptr_t envelope;

                // timeout 1000ms
                if (channel->BasicConsumeMessage(envelope, 1000)) {

                    std::string messageBody =
                        envelope->Message()->Body();

                    Poco::JSON::Parser parser;

                    auto parsedObject =
                        parser.parse(messageBody)
                              .extract<Poco::JSON::Object::Ptr>();

                    std::string eventType =
                        parsedObject->getValue<std::string>("event_type");

                    if (eventType == "booking.created") {

                        handleBookingCreated(messageBody);

                    } else if (eventType == "booking.cancelled") {

                        handleBookingCancelled(messageBody);

                    } else {

                    }

                    // Ack
                    channel->BasicAck(envelope);
                }

            } catch (const std::exception& ex) {
                std::cerr << "Consumer loop error: "<< ex.what()<< std::endl;
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(100)
                );
            }
        }

    } catch (const std::exception& ex) {
        std::cerr << "Fatal consumer error: " << ex.what() << std::endl;
        exit(1);
    }
}