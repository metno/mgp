#include <iostream>

using namespace std;

int main()
{
    float a = 3.2;
    cout << "(float = 3.2) == 3.2: " << (a == 3.2) << endl;
    cout << "(float = 3.2) == 3.2f: " << (a == 3.2f) << endl;
    a = 3.2f;
    cout << "(float = 3.2f) == 3.2: " << (a == 3.2) << endl;
    cout << "(float = 3.2f) == 3.2f: " << (a == 3.2f) << endl;

    return 0;
}
