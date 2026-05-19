#include "storage.h"
#include <algorithm>
#include <Poco/UUIDGenerator.h> 

Storage& Storage::instance() {
    static Storage s;
    return s;
}

int Storage::createUser(const std::string& login, const std::string& pass, const std::string& fn, const std::string& ln) {
    Poco::FastMutex::ScopedLock lock(_mutex);
    for (const auto& u : _users) if (u.login == login) return -1;
    User u{_nextId++, login, pass, fn, ln};
    _users.push_back(u);
    return u.id;
}

User* Storage::findUserByLogin(const std::string& login) {
    Poco::FastMutex::ScopedLock lock(_mutex);
    for (auto& u : _users) if (u.login == login) return &u;
    return nullptr;
}

std::vector<User> Storage::findUsersByMask(const std::string& mask) {
    Poco::FastMutex::ScopedLock lock(_mutex);
    std::vector<User> res;
    for (const auto& u : _users) {
        std::string full = u.firstName + " " + u.lastName;
        if (full.find(mask) != std::string::npos) res.push_back(u);
    }
    return res;
}

User* Storage::getUserById(int id) {
    Poco::FastMutex::ScopedLock lock(_mutex);
    for (auto& u : _users) if (u.id == id) return &u;
    return nullptr;
}

int Storage::createHotel(const std::string& name, const std::string& city, int stars) {
    Poco::FastMutex::ScopedLock lock(_mutex);
    Hotel h{_nextId++, name, city, stars};
    _hotels.push_back(h);
    return h.id;
}

std::vector<Hotel> Storage::getAllHotels() {
    Poco::FastMutex::ScopedLock lock(_mutex);
    return _hotels;
}

std::vector<Hotel> Storage::findHotelsByCity(const std::string& city) {
    Poco::FastMutex::ScopedLock lock(_mutex);
    std::vector<Hotel> res;
    for (const auto& h : _hotels) {
        if (h.city == city) res.push_back(h);
    }
    return res;
}

int Storage::createBooking(int userId, int hotelId, const std::string& start, const std::string& end) {
    Poco::FastMutex::ScopedLock lock(_mutex);
    Booking b{_nextId++, userId, hotelId, start, end, false};
    _bookings.push_back(b);
    return b.id;
}

std::vector<Booking> Storage::getUserBookings(int userId) {
    Poco::FastMutex::ScopedLock lock(_mutex);
    std::vector<Booking> res;
    for (const auto& b : _bookings) {
        if (b.userId == userId) res.push_back(b);
    }
    return res;
}

bool Storage::cancelBooking(int bookingId) {
    Poco::FastMutex::ScopedLock lock(_mutex);
    for (auto& b : _bookings) {
        if (b.id == bookingId) {
            b.cancelled = true;
            return true;
        }
    }
    return false;
}

std::string Storage::createSession(int userId) {
    Poco::FastMutex::ScopedLock lock(_mutex);
    Poco::UUID uuid = Poco::UUIDGenerator::defaultGenerator().createRandom();
    std::string token = uuid.toString();
    _sessions[token] = userId;
    return token;
}

int Storage::validateSession(const std::string& token) {
    Poco::FastMutex::ScopedLock lock(_mutex);
    auto it = _sessions.find(token);
    if (it != _sessions.end()) return it->second;
    return -1;
}

void Storage::destroySession(const std::string& token) {
    Poco::FastMutex::ScopedLock lock(_mutex);
    _sessions.erase(token);
}