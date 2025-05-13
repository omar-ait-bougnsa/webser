#ifndef HTTPERROR_HPP
#define HTTPERROR_HPP

#include <sstream>

#include <string>
#include <map>

class HttpError
{
private:
    int _code;
    std::string _message;

    void setMessage();

public:
    HttpError(int code);

    int getCode() const;
    std::string getMessage() const;
    std::string getStatusLine() const;
};

#endif
