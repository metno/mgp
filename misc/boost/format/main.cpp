// g++ main.cpp

#include <boost/format.hpp>
#include <iostream>

int main()
{
    std::cout << boost::format("writing %1%,  x=%2% : %3%-th try\n") % "toto" % 40.23 % 50; 
    // prints "writing toto,  x=40.230 : 50-th try"
    return 0;
}
