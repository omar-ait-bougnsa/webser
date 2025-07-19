#ifndef SESSIONDATA_HPP
#define SESSIONDATA_HPP

#include <string>


class SessionData {
private:
    std::string sessionId;
public:
    SessionData();
    SessionData(const std::string& id);

    // Getters
    const std::string& getSessionId() const;
};

#endif
