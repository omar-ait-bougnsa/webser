#include "webserver.hpp"
std::vector<std::string> tf_split(std::string line, char target)
{
	size_t pos = 0;
   int less = 0;
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

std::string check_extation (std::string path)
{
   std::string str;
   int pos = path.find_last_of(".");
   if (pos == std::string::npos)
      return "";
   path.erase(0,pos);
   std::cout <<"extation = "<< path <<std::endl;
   std::string image_extension = ".jpg .jpeg .png .gif .bmp .tiff .webp .svg";
   std::string text_extension = ".txt .html .css .js .json .xml .csv .rtf .odt .php .py .java .cpp .sql .bat";
   std::string document_extension = ".pdf .doc .docx .xls .xlsx .ppt .pptx";
   std::string audio_extension = ".mp3 .wav .ogg .flac .aac .m4a";
   std::string video_extension = ".mp4 .avi .mov .mkv .webm .flv .wmv";
   std::string archive_extension = ".zip .rar .tar .gz .7z";
   std::string executable_extension = ".apk .exe .dmg";
   path.erase(0,1);
   if (image_extension.find(path) != std::string::npos)
      str = "image/"+ path;
   else if (text_extension.find(path) != std::string::npos)
      str = "text/"+ path;
   else if (audio_extension.find(path) != std::string::npos)
      str = "audio/"+ path;
   else if (video_extension.find(path) != std::string::npos)
      str = "video/"+ path;
   else if (archive_extension.find(path) != std::string::npos)
      str = "archive" + path;
   else if (executable_extension.find(path) != std::string::npos)
      str = "executable" + path;
   return str;
}

#include <sstream>

void handel_get(int fd,std::vector<std::string> method)
{
   int size1;
   char buf[100];
   std::string str = check_extation(method[1]);
   std::string header = "HTTP/1.1 200 OK\r\nContent-Type: " + str + "\r\n\r\n";
   send(fd, header.c_str(),header.length(), 0);
   std::ifstream file(method[1].c_str(), std::ios::binary | std::ios::ate);
   size_t pos = file.tellg();
   std::stringstream ss;
   ss << file.rdbuf();
   int b;
   // return ss.str();
   if (method[1] == "/")
      b = open("index.html", O_RDONLY);
   if (b == -1)
   {
      b = open("404.html", O_RDONLY);
   }
   while (1)
   {
      size1 = read(b, buf, 100);
      if (size1 <= 0)
         break;
      send(fd, buf, size1, 0);
   }
   close (b);
   return; 
}

void handel_post(int fd,std::vector<std::string> method,std::string str)
{
   int size1;
   char buf[100];
   std::ofstream file(method[1]);
   if (file.is_open())
   {
      std::cout <<"erro\n";
   }
   const char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
   send(fd, header, strlen(header), 0);
   int b = open(method[1].c_str(), O_RDONLY);
   if (method[1].empty())
      b = open("index.html", O_RDONLY);
   if (b == -1)
      b = open("404.html", O_RDONLY);
   while (1)
   {
      size1 = read(b, buf, 100);
      if (size1 <= 0)
         break;
      send(fd, buf, size1, 0);
   }
   close (b);
}

void parst_request(int fd,char *conf_file)
{
   char buf[1024];
   std::string full_request;
   ssize_t bytes_read;
   while ((bytes_read = read(fd, buf, 1023)) > 0)
   {
      buf[bytes_read] = '\0';
      full_request += buf;
       
      if (full_request.find("\n") != std::string::npos)
         break;
   }
   int i = full_request.find("\n");
   std::string path = full_request.substr(0,i);
   std::cout <<full_request <<std::endl;
   std::vector <std::string> method = tf_split (path,' ');
   if (method[0] == "GET")
   {
      handel_get(fd,method);
   }
   else if (method[0] == "POST")
      handel_post(fd,method,full_request);
   else if (method[0] == "DELETE")
      std::cout << "this is not working\n";
}
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
void Server::pars_server(std::vector<std::string> server)
{
   int pos;
   std::pair<std::string,std::string> Pair;
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
         hostname = Pair.second;
      else if (Pair.first == "root")
         path = Pair.second;
      
   } 
}
std::vector<std::string> parst_configfile(char *filename)
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
   Server *servers = new Server[server.size()];
   for (int i=0;i < server.size();i++)
   {
       servers[i].pars_server(server[i]);
   }
   return str;
}
int main(int ac,char **av)
{
   int size1;
   struct sockaddr_in my_addr, peer_addr;
   char buf[100];
   parst_configfile(av[1]);
   // int fd = socket(AF_INET, SOCK_STREAM, 0);
   // int opt = 1;
   // if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
   //      std::cout << "setsockopt failed\n";
   // my_addr.sin_family = AF_INET;
   // my_addr.sin_port = htons(1500);
   // my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   // bind(fd, (struct sockaddr *)&my_addr, sizeof(my_addr));
   // listen(fd,20);
   // socklen_t size = sizeof(my_addr);
   // while (1)
   // {
   //    int fd1 = accept(fd, (struct sockaddr *)&my_addr, &size);
   //    parst_request (fd1,av[1]);
   //    close (fd1);
   // }
   // close(fd);
   return 0;
 }