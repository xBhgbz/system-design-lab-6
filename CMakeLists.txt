cmake_minimum_required(VERSION 3.10)
project(HotelBookingAPI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Poco REQUIRED COMPONENTS Foundation Net Util JSON)

find_path(SIMPLEAMQPCLIENT_INCLUDE_DIRS
    NAMES SimpleAmqpClient.h
    PATHS /usr/local/include/SimpleAmqpClient /usr/include/SimpleAmqpClient
    REQUIRED
)

find_library(SIMPLEAMQPCLIENT_LIBRARY
    NAMES SimpleAmqpClient
    PATHS /usr/local/lib /usr/lib
    REQUIRED
)

find_library(RABBITMQ_LIBRARY
    NAMES rabbitmq
    PATHS /usr/local/lib /usr/lib
    REQUIRED
)

find_package(Boost REQUIRED COMPONENTS system thread)

message(STATUS "SIMPLEAMQPCLIENT_INCLUDE_DIRS: ${SIMPLEAMQPCLIENT_INCLUDE_DIRS}")
message(STATUS "SIMPLEAMQPCLIENT_LIBRARY: ${SIMPLEAMQPCLIENT_LIBRARY}")
message(STATUS "RABBITMQ_LIBRARY: ${RABBITMQ_LIBRARY}")

include_directories(include ${Poco_INCLUDE_DIRS} ${SIMPLEAMQPCLIENT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

set(API_SOURCES
    src/main.cpp
    src/storage.cpp
    src/handlers.cpp
    src/auth.cpp
    src/event_producer.cpp
)

set(CONSUMER_SOURCES
    src/consumer_main.cpp
    src/event_consumer.cpp
)

add_executable(api ${API_SOURCES})
add_executable(consumer ${CONSUMER_SOURCES})

target_link_libraries(api 
    ${Poco_LIBRARIES} 
    ${SIMPLEAMQPCLIENT_LIBRARY}
    ${RABBITMQ_LIBRARY}
    ${Boost_LIBRARIES}
    pthread
    dl
)

target_link_libraries(consumer 
    ${Poco_LIBRARIES} 
    ${SIMPLEAMQPCLIENT_LIBRARY}
    ${RABBITMQ_LIBRARY}
    ${Boost_LIBRARIES}
    pthread
    dl
)