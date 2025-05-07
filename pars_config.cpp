#include "webserver.hpp"

int remove_space(std::string &line)
{
   size_t i;
   if (line.empty())
      return (0);

   i = line.size() -1;
   while (i > 0)
   {
      if (!isspace(line[i]))
      {
         line.erase(i +1,line.size() - i);
         break;
      }
      i--;
   }
   i = 0;
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
Route Route::pars_Route(std::vector<std::string>location)
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
            throw ("error in root");
         root_directory = v[0];
      }
      else if (str  == "location")
      {
         if (v.size() != 1)
            throw ("Error invalid location");
         path_prefix = v[0];
      }
      else if (str  == "autoindex")
      {
         if (v.size() != 1 || (v[0] != "on" && v[0] != "off"))
            throw ("error in autoindex must 'on' or 'of'");
         path_prefix = v[0];
      }
      else if (str == "methods")
          methods = v;
      else if (str == "cgi")
         cgi = v;
      else if (str == "cgi_path")
         cgi_path = v;
      else if (str == "}" && v.size() == 1)
         continue;
      else
         throw ("Error in logic location");

      i++;
   }
   return *this;
}

void Server::prse_error_page(std::string str)
{
   std::pair<int,std::string> Pair;
   long int nb;
   char * pos;
   std::vector<std::string> v = split_withspace (str);
   size_t i = 0;
   if (v.size() < 2)
   {
      std::cout <<"error\n";
      exit (1);
   }
   while (i < v.size() - 1)
   {
      nb = strtoll(v[i].c_str(), &pos, 10);
      if (pos[0] == '\0')
      {
         Pair.first = nb;
         Pair.second = v[v.size() - 1];
         error_page.insert(Pair);
      }
      else
      {
         std::cout <<"error in page error\n";
         exit (1);
      }
      i++;
   }

}
void check_bracket(std::vector<std::string> &server)
{
   size_t i = 0;
   int bracket = 0;
   size_t pos;
   while (i < server.size())
   {
      pos = server[i].find("{");
      if (pos != std::string::npos)
      {
         bracket += 1;
         server[i].erase(pos,1);
      }
      pos = server[i].find("}");
      if (pos != std::string::npos)
         bracket -= 1;
      i++;
   }
   if (bracket != 0)
      throw ("Error bracket not closed\n");
}
void Server::pars_server(std::vector<std::string> server,int size)
{
   size_t pos;
   char *end;
   (void)size;
   std::pair<std::string,std::string> Pair;
   std::vector<std::string>str;
   check_bracket(server);
   for (size_t i = 0; i < server.size();i++)
   {
      if ((pos = server[i].find (" ")) != std::string::npos || (pos = server[i].find ("   ")) != std::string::npos)
      {
         Pair.first = server[i].substr(0,pos);
         Pair.second = server[i].substr (pos,server[i].length() - pos);
      }
      if (Pair.first == "location")
      {
        while (server[i].find("}") == std::string::npos)
        {
            str.push_back(server[i]);
            i++;
        }
        i++;
      routes.push_back(pars_Route (str));
      }
      if (Pair.first == "listen")
         port = Pair.second;
      else if (Pair.first == "server_name")
         server_names = split_withspace(Pair.second);
      else if (Pair.first == "error_page")
         prse_error_page(Pair.second);
      else if (Pair.first == "client_max_body_size")
      {
         size_t nb = strtoll(Pair.second.c_str(), &end, 10);
         if (end[0] == 'M' && end[1] == '\0')
            client_max_body_size = nb;
         else
            std::cout <<"\n\n\nend = " <<end[0]<<std::endl;
      }
      Pair.first = "", Pair.second = "";
   }
}

std::vector<Server> parst_configfile(char *filename)
{
   std::ifstream file (filename);
   std::vector<std::vector<std::string> > server;
   std::vector<std::string> str;
   bool check_server = false;
   size_t pos;
   if (!file.is_open())
   {
      std::cout <<"can't open Configuration file\n";
      exit(1);
   }
   std::string line;
   while (getline(file,line))
   {
      pos = line.find("#");
      if (pos != std::string::npos)
      {
         line.erase(pos,line.length() - pos);
      }
      pos = line.find("server");
      if (pos  != std::string::npos && !str.empty() && !check_server)
            throw ("Error config file must start 'server'");
      if (pos  != std::string::npos)
      {
         check_server = true;
         if (!str.empty() && (line[pos + 6] == '\0' || isspace(line[pos + 6]) || line[pos + 6] == '{'))
         {
            str.push_back(line);
            server.push_back(str);
            str.clear();
         }
      }
      if(remove_space(line))
      {
         if (line[line.size() -1] == ';')
            line.erase(line.size() -1,1);
         str.push_back(line);
      }
   }
   server.push_back(str);
   Server servers;
   std::vector<Server> v;
   for (size_t i = 0;i < server.size();i++)
   {
       servers.pars_server(server[i],server.size());
       v.push_back(servers);
   }
   return v;
}