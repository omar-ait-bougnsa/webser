#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include "HttpRequest.hpp"

class Validator
{
private:
    int     _status_code;
public:
    Validator();
    ~Validator();

    bool validateRequestLine(const HttpRequest& request);
    bool validateHeaders(const HttpRequest& request);
    bool validate(const HttpRequest& request, int &status);
};

#endif
