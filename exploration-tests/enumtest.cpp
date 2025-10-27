#include <iostream>

enum level
{
    LOW,
    MEDIUM,
    HIGH,

};

int main()
{
    enum level myVar;
    enum level mYvar = MEDIUM;
    std::cout << myVar;
}