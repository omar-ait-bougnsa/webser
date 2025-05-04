#include <iostream>
#include <cstdlib>  // for strtoll
#include <cerrno>   // for errno
#include <climits>  // for LLONG_MAX and LLONG_MIN

int main() {
    const char *str = "1239999999999999999999999999999999999999945abc";
    char *end;
    errno = 0;  // reset errno

    long long result = strtoll(str, &end, 10);

    if (errno == ERANGE) {
        std::cerr << "Out of range!" << std::endl;
    } else {
        std::cout << "Parsed number: " << result << std::endl;
        std::cout << "Rest of string: " << end << std::endl;
    }

    return 0;
}