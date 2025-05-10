#include "webserver.hpp"
std::vector<std::string> split(std::string line, char target)
{
	size_t pos = 0;
	std::vector <std::string> str;
	while (1)
	{
		pos = line.find (target);
		if (pos == std::string::npos)
        {
            str.push_back(line.substr (0,line.length()));
			   break;
        }
      str.push_back(line.substr (0,pos));
		line.erase(0,pos + 1);
	}
   return str;
}
std::vector<std::string> split_withspace(std::string str)
{
   std::vector<std::string> arrys;
   std::stringstream ss;
   ss <<str;
   while (ss >> str)
   {
      arrys.push_back (str);
   }
   return arrys;
}

void parst_request(int fd)
{
   char buf[10000];
   std::string full_request;
   ssize_t bytes_read;
   while ((bytes_read = read(fd, buf, 9999)) > 0)
   {
      buf[bytes_read] = '\0';
      full_request += buf;
      break;
   }
   int i = full_request.find("\n");
   std::string path = full_request.substr(0,i);
   std::vector <std::string> method = split (path,' ');
   // size_t pos = full_request.find("\r\n\r\n");
   // if (pos != std::string::npos)
   // full_request.erase(0,pos);
   //std::cout <<full_request <<std::endl;
   if (method[0] == "GET")
      handel_get(fd,method);
   else if (method[0] == "POST")
      handel_post(fd,method,full_request);
   else if (method[0] == "DELETE")
      std::cout <<"baki makhdam\n";
}

int main(int ac,char **av)
{
   
   if (ac != 2)
      exit (1);
   std::vector <Server> server;
   struct sockaddr_in my_addr;
   try
   {
      server = parst_configfile(av[1]);
   }
   catch(const char * e)
   {
      std::cout << e << '\n';
      return 0;
   }
   
   size_t i =0;
   while (i < server.size())
   {
     // server[i].printServer();
      i++;
   }
   int fd = socket(AF_INET, SOCK_STREAM, 0);
   int opt = 1;
   if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        std::cout << "setsockopt failed\n";
   my_addr.sin_family = AF_INET;
   my_addr.sin_port = htons(1500);
   my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   bind(fd, (struct sockaddr *)&my_addr, sizeof(my_addr));
   listen(fd,20);
   socklen_t size = sizeof(my_addr);
   while (1)
   {
      int fd1 = accept(fd, (struct sockaddr *)&my_addr, &size);
      std::cout <<"llll\n";
      parst_request (fd1);
      close (fd1);
   }
   close(fd);
   return 0;
 }