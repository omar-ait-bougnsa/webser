#include "../include/Route.hpp"


Route Route::pars_Route(std::vector<std::string>location)
{
   size_t i = 0;

   std::vector<std::string>v;
   std::string str;
   while (i < location.size())
   {
      v = Tools::split_withspace(location[i]);
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
         autoindex = v[0];
      }
      else if (str == "methods")
          methods = v;
      else if (str == "cgi")
         cgi = v;
      else if (str == "cgi_path")
         cgi_path = v;
      else
         throw ("Error: some value in location not valid");
      i++;
   }
   return *this;
}



void Route::printRoute() const {
    std::cout << "Route Configuration:\n";
    std::cout << "  Path Prefix: " << path_prefix << "\n";
    std::cout << "  Root Directory: " << root_directory << "\n";
    std::cout << "  Autoindex: " << autoindex << "\n";
    std::cout << "  Methods: ";
    
    // For-loop compatible with C++98:
    for (size_t i = 0; i < methods.size(); ++i)
        std::cout << methods[i] << " ";
    std::cout << "\n";

    std::cout << "  Index Files: ";
    // For-loop compatible with C++98:
    for (size_t i = 0; i < index_files.size(); ++i)
        std::cout << index_files[i] << " ";
    std::cout << "\n";
}
