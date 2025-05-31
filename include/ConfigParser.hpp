#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <vector>
#include "./VirtualHost.hpp"

class ConfigParser
{
private:
    std::string                     _fileName;
    std::vector<VirtualHost>    _virtualHosts;

public:
    ConfigParser(const std::string& filename);

    std::vector<VirtualHost>        parse();
    int                             remove_space(std::string &line);

    
    // const std::vector<VirtualHost> &getVirtualHosts() const;
};

#endif

