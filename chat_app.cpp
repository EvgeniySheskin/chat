#include <iostream>
#include "UserManager.h"
#include "NoSuchUserException.h"
#include "NewUserException.h"
#include "Menu.h"

void func(string a, string ...)
{

}


int main()
{
    bool success = true;
    cout << "Welcome to the ChatApp!\n\n";

    auto welcomeMenu = new Menu<int>();
    welcomeMenu->AddItem(new MenuItem<int>("Register new user (sign up)", 1, func), 0)->
        AddItem(new MenuItem<int>("Enter with existing account (sign in)", 2, func), 1)->
        AddItem(new MenuItem<int>("Exit", 0, func), 2);
    cout << *welcomeMenu;
    cout << "Please, choose one of the following options:\n";
    

/*    do
    {
        cout << "Hello World!\n";

    } while (success);*/
    
}