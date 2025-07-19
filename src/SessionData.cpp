#include "../include/SessionData.hpp"


SessionData::SessionData(): sessionId("")
{}

SessionData::SessionData(const std::string& id) : sessionId(id) {}

const std::string& SessionData::getSessionId() const {
    return sessionId;
}
