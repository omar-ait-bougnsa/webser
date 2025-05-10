#include "webserver.hpp"


// void handel_post(int fd,std::vector<std::string> method,std::string request)
// {
//    std::stringstream ss;
//    (void)request;
//    const char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
//    send(fd, header, strlen(header), 0);
//    std::ifstream file;
//    file.open(method[1].c_str());
//    if (method[1] == "/")
//       file.open("index.html");
//    if (file.is_open())
//       file.open("404.html");
//    ss << file.rdbuf();
//    file.seekg(0, file.end);
//    size_t pos = file.tellg();
//    std::string r = "yes all it is good\n";
//    send(fd,r.c_str(),pos, 0);
// }

void handel_post(int fd, std::vector<std::string> method, std::string request)
{
   HttpResponse post;
   size_t pos = request.find("\r\n\r\n");
   if (pos != std::string::npos)
      post.body = request.substr (pos +5,request.length() - 1);
   
   std::cout <<post.body <<std::endl;
   (void)method;
    std::string body =  "{\"txt\":\"Upload successful and all it is good\"}";
    std::stringstream ss;
    ss << "HTTP/1.1 200 OK\r\n";
    ss << "Content-Type: txt/txt\r\n";
    ss << "Content-Length: " << body.size() << "\r\n";
    ss << "Connection: close\r\n\r\n";
    ss << body;
    
    std::string response = ss.str();
    send(fd, response.c_str(), response.size(), 0);
}
std::string check_extation (std::string path)
{
   std::string str;
   size_t pos = path.find_last_of(".");
   if (pos == std::string::npos)
      return "";
   path.erase(0,pos);
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
      file.open("404.html", std::ios::binary);
   ss << file.rdbuf();
   file.seekg(0, file.end);
   size_t pos = file.tellg();
   send(fd, ss.str().c_str(), pos, 0);
   file.close();
   return; 
}
