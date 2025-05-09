#include <iostream>
#include <cstdlib>  // for strtoll
#include <cerrno>   // for errno
#include <climits>  // for LLONG_MAX and LLONG_MIN

int main() {
  float a = 1.0/0.0;
  std::cout <<"a =" <<a <<std::endl;
    return 0;
}