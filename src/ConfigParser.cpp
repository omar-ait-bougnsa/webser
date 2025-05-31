#include "../include/ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& filename): _fileName(filename) {}

std::vector<VirtualHost> ConfigParser::parse()
{
    std::ifstream file(_fileName.c_str());
    std::vector<std::vector<std::string> > server;
    std::vector<std::string> str;
    bool check_server = false;
    size_t pos;
    if (!file.is_open())
    {
        std::cout << "can't open Configuration file\n";
        exit(1);
    }
    std::string line;
    while (getline(file, line))
    {
        pos = line.find("#");
        if (pos != std::string::npos)
        {
            line.erase(pos, line.length() - pos);
        }
        if (line == "server" && !str.empty() && !check_server)
            throw("Error config some line is not in block server ");

        if (remove_space(line) && line != "server")
        {
            if (line[line.size() - 1] == ';')
                line.erase(line.size() - 1, 1);
            str.push_back(line);
        }
        if (line == "server" && !str.empty())
        {
            check_server = true;
            server.push_back(str);
            str.clear();
        }
    }
    
    server.push_back(str);
    VirtualHost host;

    for (size_t i = 0; i < server.size(); i++)
    {
        host.pars_server(server[i], server.size());
        _virtualHosts.push_back(host);
    }
    return _virtualHosts;
}


int     ConfigParser::remove_space(std::string &line)
{
    size_t i;
    if (line.empty())
        return (0);
    i = line.size() - 1;
    while (i > 0)
    {
        if (!isspace(line[i]))
        {
            line.erase(i + 1, line.size() - i);
            break;
        }
        i--;
    }
    i = 0;
    while (line[i])
    {
        if (!isspace(line[i]))
        {
            line.erase(0, i);
            return (1);
        }
        i++;
    }
    return 0;
}