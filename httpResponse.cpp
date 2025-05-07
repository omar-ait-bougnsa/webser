#include "webserver.hpp"

void handel_post(int fd,std::vector<std::string> method,std::string request)
{
   std::stringstream ss;
   (void)request;
   const char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
   send(fd, header, strlen(header), 0);
   std::ifstream file;
   file.open(method[1].c_str());
   if (method[1] == "/")
      file.open("index.html");
   if (file.is_open())
      file.open("404.html");
   ss << file.rdbuf();
   file.seekg(0, file.end);
   size_t pos = file.tellg();
   // std::string 
   send(fd,ss.str().c_str(),pos, 0);
}