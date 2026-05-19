#pragma once
#include <Poco/Mutex.h>
#include <Poco/UUID.h>
#include <vector>
#include <map>
#include <string>

struct User {
    int id;
    std::string login;
    std::string password; 
    std::string firstName;
    std::string lastName;
};

struct Hotel {
    int id;
    std::string name;
    std::string city;
    int stars;
};

struct Booking {
    int id;
    int userId;
    int hotelId;
    std::string startDate;
    std::string endDate;
    bool cancelled;
};

class Storage {
public:
    static Storage& instance();
    
    // User
    int createUser(const std::string& login, const std::string& pass, const std::string& fn, const std::string& ln);
    User* findUserByLogin(const std::string& login);
    std::vector<User> findUsersByMask(const std::string& mask);
    User* getUserById(int id);

    // Hotel
    int createHotel(const std::string& name, const std::string& city, int stars);
    std::vector<Hotel> getAllHotels();
    std::vector<Hotel> findHotelsByCity(const std::string& city);

    // Booking
    int createBooking(int userId, int hotelId, const std::string& start, const std::string& end);
    std::vector<Booking> getUserBookings(int userId);
    bool cancelBooking(int bookingId);

    // Auth
    std::string createSession(int userId);
    int validateSession(const std::string& token);
    void destroySession(const std::string& token);

private:
    Storage() {}
    Poco::FastMutex _mutex;
    std::vector<User> _users;
    std::vector<Hotel> _hotels;
    std::vector<Booking> _bookings;
    std::map<std::string, int> _sessions; 
    int _nextId = 1;
};