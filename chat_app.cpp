#include <iostream>
#include "UserManager.h"
#include "NoSuchUserException.h"
#include "NewUserException.h"
#include "Menu.h"

int main()
{
    bool success = true;
    cout << "Welcome to the ChatApp!\n\n";

    auto userManager = new UserManager();
    auto welcomeMenu = new Menu<int, UserManager>();
    // формируем начальное меню
    welcomeMenu->
    AddItem(new MenuItem<int, UserManager>("Register new user (sign up)", 1,
    [manager = userManager]()
    {
        string login, password, nickname;
        bool condition = true;
        do
        {
            cout << "Enter user login: " << endl;
            cin >> login;
            if (manager->CheckLogin(login))
            {
                condition = false;
            }
        } while (condition);
        condition = true;
        do
        {
            cout << "Enter user password: " << endl;
            cin >> password;
            if (manager->CheckPassword(password))
            {
                condition = false;
            }
        } while (condition);
        cout << "Enter user name (available to others): " << endl;
        cin >> nickname;
        manager->AddNewUser(login, password, nickname);
    }), 0)->
    AddItem(new MenuItem<int, UserManager>("Enter with existing account (sign in)", 2,
    [userManager]()
    {
        string login, password, nickname;
        bool condition = true;
        do
        {
            cout << "Enter user login: " << endl;
            cin >> login;
            if (userManager->CheckLogin(login))
            {
                condition = false;
            }
        } while (condition);
        condition = true;
        do
        {
            cout << "Enter user password: " << endl;
            cin >> password;
            if (userManager->CheckPassword(password))
            {
                condition = false;
            }
        } while (condition);
        cout << "Enter user name (available to others): " << endl;
        cin >> nickname;
        userManager->AddNewUser(login, password, nickname);
    }), 1)->
    AddItem(new MenuItem<int, UserManager>("Exit", 0,
    [userManager]()
    {
        cout << "Have a nice day!\n\n";
        exit(0);
    }), 2);
        
    while (true)
    {
        bool success = true;
        int code = -1;
        system("cls");
        cout << *welcomeMenu;
        cout << "Please, choose one of the following options:\n";
        cin >> code;
        success = welcomeMenu->Execute(code);
        if (success)
        {
            break;
        }
        else
        {
            cout << "There's no such option! Please, try again.\n";
        }
    }


    
}