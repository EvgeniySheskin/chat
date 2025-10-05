#include <iostream>
#include "ChatApp.h"

using namespace chat;


int main()
{
    std::setlocale(LC_ALL, "");
    ChatApp app;
    app.Run();
    return 0;
    
}