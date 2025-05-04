#include "webserver.hpp"
std::vector<std::string> split(std::string line, char target)
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


void handel_get(int fd,std::vector<std::string> method)
{
   int size1;
   char buf[100];
   std::string str = check_extation(method[1]);
   std::string header = "HTTP/1.1 200 OK\r\nContent-Type: " + str + "\r\n\r\n";
   send(fd, header.c_str(),header.length(), 0);
   std::ifstream file;
   if (method[1] == "/")
      file.open("index.html", std::ios::binary);
   else
      file.open(method[1].c_str(), std::ios::binary);
   std::stringstream ss;
   if (!file.is_open())
   {
      file.open("404.html", std::ios::binary);
   }
   ss << file.rdbuf();
   file.seekg(0, file.end);
   size_t pos = file.tellg();
   send(fd, ss.str().c_str(), pos, 0);
   file.close();
   return; 
}

void handel_post(int fd,std::vector<std::string> method,std::string request)
{
   int size1;
   char buf[100];
   const char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
   send(fd, header, strlen(header), 0);
   std::ofstream file;
   file.open(method[1]);
   if (method[1] == "/")
      file.open("index.html");
   if (file.is_open())
      file.open("404.html");
   size_t pos = request.find ("\r\n\r\n");
   if (pos == std::string::npos)
         std::cout << "error\n";
   // std::string 
   send(fd, buf, size1, 0);
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
   std::vector <std::string> method = split (path,' ');
   if (method[0] == "GET")
      handel_get(fd,method);
   else if (method[0] == "POST")
      handel_post(fd,method,full_request);
   else if (method[0] == "DELETE")
      std::cout <<"baki makhdam\n";
}


int main(int ac,char **av)
{
   int size1;
   std::vector <Server> server;
   struct sockaddr_in my_addr, peer_addr;
   char buf[100];
   server = parst_configfile(av[1]);
   int i =0;
   while (i < server.size())
   {
      server[i].print();
   }
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