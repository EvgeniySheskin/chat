#include "ChatClient.h"
#include <iostream>
#include <cstdlib> // для std::atoi

// Значения по умолчанию
#ifndef PORT_CLIENT
#define PORT_CLIENT 7777
#endif

#ifndef IP_REMOTE
#define IP_REMOTE "127.0.0.1"
#endif

int main(int argc, char* argv[]) 
{
#ifdef _WIN32
    //system("chcp 65001 > nul"); // UTF-8
    //setlocale(LC_ALL, "");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#else
    setlocale(LC_ALL, "ru_RU.UTF-8");
#endif

    std::string serverIp = IP_REMOTE;
    int port = PORT_CLIENT;

    if (argc >= 2) 
    {
        serverIp = argv[1];

        if (argc >= 3) 
        {
            try 
            {
                int tempPort = std::atoi(argv[2]);
                if (tempPort >= 1024 && tempPort < 65536) 
                {
                    port = tempPort;
                }
                else 
                {
                    std::cerr << "Ошибка: порт должен быть числом от 1024 до 65535.\n";
                    return 1;
                }
            }
            catch (...) 
            {
                std::cerr << "Ошибка: порт должен быть числом от 1024 до 65535.\n";
                return 1;
            }
        }
    }

    chat::ChatClient client(serverIp, port);
    client.run();

    return 0;
}