#include <iostream>
#include "UserManager.h"
#include "NoSuchUserException.h"
#include "NewUserException.h"
#include "Menu.h"

using namespace chat;


bool ShowMenu(Menu<>* menu)
{
    int code = -1;
    std::cout << *menu;
    std::cout << "Please, choose one of the following options:\n";
    std::cin >> code;
    return menu->Execute(code);
}

Menu<>* CreateWelcomeMenu(UserManager* um)
{

}


int main()
{
    Menu<>* activeMenu;
    bool success = true;
    std::cout << "Welcome to the ChatApp!\n\n";

    auto userManager = new UserManager();
    auto welcomeMenu = new Menu<int>();
    // формируем начальное меню
    welcomeMenu->
    AddItem(new MenuItem<int>("Register new user (sign up)", 1,
    [manager = userManager]()
    {
        string login, password, nickname;
        bool condition = true;
        do
        {
            std::cout << "Enter user login: " << endl;
            std::cin >> login;
            if (manager->CheckLogin(login))
            {
                condition = false;
            }
        } while (condition);
        condition = true;
        do
        {
            std::cout << "Enter user password: " << endl;
            std::cin >> password;
            if (manager->CheckPassword(password))
            {
                condition = false;
            }
        } while (condition);
        std::cout << "Enter user name (available to others): " << endl;
        std::cin >> nickname;
        manager->AddNewUser(login, password, nickname);
    }, welcomeMenu), 0)->
    AddItem(new MenuItem<int>("Enter with existing account (sign in)", 2,
    [userManager]()
    {
        string login, password, nickname;
        bool condition = true;
        do
        {
            std::cout << "Enter user login: " << endl;
            std::cin >> login;
            if (userManager->CheckLogin(login))
            {
                condition = false;
            }
        } while (condition);
        condition = true;
        do
        {
            std::cout << "Enter user password: " << endl;
            std::cin >> password;
            if (userManager->CheckPassword(password))
            {
                condition = false;
            }
        } while (condition);
        std::cout << "Enter user name (available to others): " << endl;
        std::cin >> nickname;
        userManager->AddNewUser(login, password, nickname);
    }, welcomeMenu), 1)->
    AddItem(new MenuItem<int>("Exit", 0,
    [userManager]()
    {
        std::cout << "Have a nice day!\n\n";
        exit(0);
    }, welcomeMenu), 2);
        
    
    while (true)
    {
        if (ShowMenu(activeMenu))
        {
            system("cls");
        }
        else
        {
            system("cls");
            cout << "No such option, choose differently: \n";
        }
    }
}