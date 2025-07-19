#include "../include/Route.hpp"

void Route::clear ()
{
   path_prefix.clear();
   root_directory.clear();
   autoindex.clear();
   methods.clear();
   index_files.clear();
   cgi.clear();
   cgi_path.clear();
   redirect_code.clear();
   redirect_location.clear();
}

Route Route::pars_Route(std::vector<std::string>location)
{
   size_t i = 0;
   std::vector<std::string>   v;
   std::string                str;

   while (i < location.size())
   {
      v = Tools::split_withspace(location[i]);
      str = v[0];
      v.erase(v.begin());
      if (str == "index")
      {
         if (index_files.size() > 0)
            throw std::logic_error ("Error: duplicate index files");
         index_files = v;
      }
      else if (str  == "root")
      {
         if(root_directory.size() > 0)
            throw std::logic_error("Error: duplicate root path");
         if (v.size() != 1)
            throw std::logic_error("Error: in root path");
         root_directory = v[0];
      }
      else if (str  == "location")
      {
         if (v.size() != 1)
            throw std::logic_error("Error invalid location");
         path_prefix = v[0];
      }
      else if (str  == "autoindex")
      {
         if (autoindex.size() > 0)
            throw std::logic_error("Error: duplicate autoindex");
         if (v.size() != 1 || (v[0] != "on" && v[0] != "off"))
            throw std::logic_error("error in autoindex must 'on' or 'of'");
         autoindex = v[0];
      }
      else if (str == "methods")
         methods = v;
      else if (str == "cgi")
      {
         if (cgi.size() > 0)
            throw std::logic_error("Error: duplicate cgi");
         if (v.size() != 2)
            throw std::logic_error("Error: cgi must have two values");
         cgi.push_back(v[0]);
         cgi_path.push_back(v[1]);
      }
      else if (str == "upload_path")
      {
         if (upload_path.size() > 0)
            throw std::logic_error("Error: duplicate upload path");
         if (v.size() != 1)
            throw std::logic_error("Error: in upload path");
         upload_path = v[0];
      }
      else if (str == "redirect")
      {
         if (redirect_code.size() > 0 || redirect_location.size() > 0)
            throw std::logic_error("Error: duplicate redirect");
         if (v.size() != 2)
            throw std::logic_error("Error: redirect must have two values");
         redirect_code = v[0];
         redirect_location = v[1];
      }
      else if (str == "return")
      {
         if (redirect_code.size() > 0)
            throw std::logic_error("Error: duplicate return");
         if(v.size() != 2)
            throw std::logic_error("Error: return must have two values");
         redirect_code = v[0];
         redirect_location = v[1];
      }
      else
         throw std::logic_error("Error: some value in location not valid");
      i++;
   }
   if (methods.empty())
   {
      methods.push_back("GET");
      methods.push_back("POST");
      methods.push_back("DELETE");
   }
   return *this;
}