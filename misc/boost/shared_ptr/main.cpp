// g++ main.cpp

#include <iostream>
#include <boost/shared_ptr.hpp>

using namespace std;

class A
{
public:
    A()  { cerr << "A:: A() (" << this << ") ...\n"; }
    ~A() { cerr << "A::~A() (" << this << ") ...\n"; }
    void f() { cerr << "A::f() ...\n"; }
};

int main()
{
    A *pa = new A;

    cerr << "ok1, pa:" << pa << endl;

    boost::shared_ptr<A> spa2;

    {
        boost::shared_ptr<A> spa1(pa);
        cerr << "spa1.use_count(): " << spa1.use_count() << endl; // prints 1

        spa1->f();

        cerr << "spa2.use_count(): " << spa2.use_count() << endl; // prints 0
        spa2 = spa1;
        cerr << "spa2.use_count(): " << spa2.use_count() << endl; // prints 2
        cerr << "spa1.use_count(): " << spa1.use_count() << endl; // prints 2
    }

    cerr << "spa2.use_count(): " << spa2.use_count() << endl; // prints 1

    cerr << "ok2\n";

    return 0;
}
