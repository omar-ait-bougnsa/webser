#include "webserver.hpp"

int is_space(std::string &line)
{
   int i = 0;
   if (line.empty())
      return (0);
   while (line[i])
   {
      if (!isspace(line[i]))
      {
         line.erase(0,i);
         return (1);
      }
      i++;
   }
   return 0;
}
void Server::pars_Route(std::vector<std::string>location)
{
   size_t i = 0;
   std::vector<std::string>v;
   std::string str;
   while (i < location.size())
   {
      v = split_withspace(location[i]);
      str = v[0];
      v.erase(v.begin());
      if (str == "index")
            index_files = v;
      else if (str  == "root")
      {
         if (v.size() != 1)
         {
            std::cout <<"error root\n";
            exit(1);
         }
         root_directory = v[0];
      }
      else if (str  == "location")
      {
         if (v.size() != 1)
         {
            std::cout <<"error location\n";
            exit(1);
         }
         path_prefix = v[0];
      }
      else if (str  == "autoindex")
      {
         if (v.size() != 1)
         {
            std::cout <<"error autoindex \n";
            exit(1);
         }
         path_prefix = v[0];
      }
      else if (str == "methods")
          methods = v;
      i++;
   }
}
void Server::pars_server(std::vector<std::string> server,int size)
{
   int pos;
   std::pair<std::string,std::string> Pair;
   std::vector<std::string>str;
   for (int i = 0; i < server.size();i++)
   {
      if ((pos = server[i].find (" ")) != std::string::npos || (pos = server[i].find ("   ")) != std::string::npos)
      {
         Pair.first = server[i].substr(0,pos);
         Pair.second = server[i].substr (pos,server[i].length() - pos);
      }
      if (Pair.first == "listen")
         port = Pair.second;
      else if (Pair.first == "server_name")
         doman_name = Pair.second;
      else if (Pair.first == "location")
      {
        while (server[i].find("}") == std::string::npos)
        {
            str.push_back(server[i]);
            i++;
        }
        pars_Route (str);
      }
      Pair.first = "", Pair.second = "";
   }
}

std::vector<Server> parst_configfile(char *filename)
{
   std::ifstream file (filename);
   std::vector<std::vector<std::string> > server;
   std::vector<std::string> str;
   int pos;
   if (!file.is_open())
   {
      std::cout <<"can't open Configuration file\n";
      exit(1);
   }
   std::string line;
   while (getline(file,line))
   {
      if (pos = line.find("#") != std::string::npos)
      {
         line.erase(pos,line.length() - pos);
      }
      if ((pos = line.find("server")) != std::string::npos && !str.empty())
      {
         if (line[pos + 6] == '\0' || isspace(line[pos + 6]) || line[pos + 6] == '{')
         {
            server.push_back(str);
            str.clear();
         }
      }
      if(is_space(line))
      {
         str.push_back(line);
      }
   }
   server.push_back(str);
   Server servers;
   std::vector<Server> v;
   for (int i=0;i < server.size();i++)
   {
       servers.pars_server(server[i],server.size());
       v.push_back(servers);
   }
   return v;
}