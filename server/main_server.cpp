#include "ChatServer.h"
#include <iostream>


#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void killProcessOnPort(int port) 
{
    char command[256];
    snprintf(command, sizeof(command), "lsof -t -i:%d -sTCP:LISTEN", port);
    FILE* fp = popen(command, "r");
    if (fp) 
    {
        char pid_str[16];
        pid_t current_pid = getpid();

        while (fgets(pid_str, sizeof(pid_str), fp) != nullptr) 
        {
            pid_str[strcspn(pid_str, "\n")] = 0;
            int pid = atoi(pid_str);

            if (pid > 0 && pid != current_pid) 
            {
                std::cout << "Завершение процесса с PID " << pid << ", использующего порт " << port << std::endl;

                if (kill(pid, SIGTERM) == 0) 
                {
                    for (int i = 0; i < 30; ++i) 
                    {
                        if (kill(pid, 0) != 0) 
                        {
                            break;
                        }
                        usleep(100000);
                    }

                    if (kill(pid, 0) == 0) 
                    {
                        std::cout << "SIGTERM не сработал, отправляем SIGKILL..." << std::endl;
                        kill(pid, SIGKILL);
                    }
                }
                else 
                {
                    kill(pid, SIGKILL);
                }
            }
        }
        pclose(fp);
    }

    // Ждём освобождения сокета
    usleep(500000); // 500 мс
}
#endif

int main() 
{
#ifdef _WIN32
    //system("chcp 65001 > nul"); // UTF-8
    //setlocale(LC_ALL, "ru_RU.UTF-8");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#else
    setlocale(LC_ALL, "ru_RU.UTF-8");
#endif
    try
    {
#ifdef __linux__
        killProcessOnPort(PORT_SRV); // Убиваем процесс на порту
#endif
        chat::ChatServer server(PORT_SRV);
        //std::cout << "Сервер запущен...\n";
        server.run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Ошибка сервера: " << e.what() << std::endl;
        return 1;
    }
}