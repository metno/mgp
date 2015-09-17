// This example shows how accumulation of an object state can be expressed
// as a chain of parenthesis expressions.

#include <iostream>

class A
{
public:
    A () : x_(0), y_(0) { }
    A (int x, int y) : x_(x), y_(y)  { }
    A (int x, const std::string &s, int y) : x_(x), s_(s), y_(y)  { }

    A &f() { return *this; }

    A &operator()(int x, int y)
    {
        x_ += x;
        y_ += y;
        return *this;
    }

    A &operator()(int x, const std::string &s, int y)
    {
        x_ += x;
        y_ += y;
        s_ += s;
        return *this;
    }

    int x_, y_;
    std::string s_;
};

int main()
{
    A a;
    a.f() (5, 6) (2, "foo", 2) (8, 1) (0, "bar", 0);

    std::cerr << a.x_ << " " << a.y_ << " >" << a.s_ << "<" << std::endl;

    return 0;
}
