#include <memory>
#include <map>
#include <iostream>

using namespace std;

class A
{
public:
    A() { cerr << "A() ... " << this << endl; }
    ~A() { cerr << "~A() ..." << this << endl; }
};

int main()
{
    map<string, shared_ptr<A> > m;
    m["aaa"] = shared_ptr<A>(new A);
    m["bbb"] = shared_ptr<A>(new A);

    return 0;
}
