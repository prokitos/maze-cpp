#include <memory>
#include <span>
#include <vector>
#include <any>
#include <algorithm>
#include <cstring>
#include <iostream>
#include "render.h"

using std::cout;
using std::string;

void consoleClear()
{

    for (short i = 0; i < 10; i++)
    {
        cout << "\t\n"; 
    }
    
}



int main()
{
    consoleClear();

    mainEngine();

    return 0;
};

