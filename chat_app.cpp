#include <iostream>
#include <windows.h>
#include "ChatApp.h"

using namespace chat;


int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    ChatApp app;
    app.Run();
    return 0;
}