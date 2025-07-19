#include "../include/Tools.hpp"

Tools::Tools()
{
}

Tools::~Tools()
{
}

std::string Tools::readFile(const std::string &filePath)
{
    std::cout << " inside readFile() : filename: " << filePath << std::endl;
    std::ifstream file(filePath.c_str());
    if (!file.is_open())
    {
        std::cout << "error ll\n";
        return ""; // Or handle error
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
std::string Tools::int_to_string(int num)
{
    std::ostringstream oss;
    oss << num;
    return oss.str();
}
std::vector<std::string> Tools::split(std::string line, char target)
{
    size_t pos = 0;
    std::vector<std::string> str;
    while (1)
    {
        pos = line.find(target);
        if (pos == std::string::npos)
        {
            str.push_back(line.substr(0, line.length()));
            break;
        }
        str.push_back(line.substr(0, pos));
        line.erase(0, pos + 1);
    }
    return str;
}

std::vector<std::string> Tools::split_withspace(std::string str)
{
    std::vector<std::string> arrys;
    std::stringstream ss;
    ss << str;
    while (ss >> str)
    {
        arrys.push_back(str);
    }
    return arrys;
}