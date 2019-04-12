#include <iostream>
#include "ChatClient.hpp"

using namespace std;

static void Usage(string proc){
    cout << "Usage: " << proc << "peer_ip" << endl;
}

static void Menu(int &s){
    cout << "______________________________________" << endl;
    cout << "1. Register      2. Login     3. Exit" << endl;
    cout << "______________________________________" << endl;
    cout << "Please Select:" << endl;
    cin >> s;
}

int main(int argc, char* argv[])
{
    if(argc != 2){
        Usage(argv[0]);
        exit(1);
    }
    ChatClient *cp = new ChatClient(argv[1]);
    cp->InitClient();

    int select = 0;
    while(1){

        Menu(select);
        switch(select){
            case 1:
                cp->Register();
                break;
            case 2:
                if(cp->Login()){
                    cp->Chat();
                }
                else{
                  cout << "Login Failed!" << endl;  
                }
                break;
            case 3:
                exit(0);
                break;
            default:
                exit(1);
                break;
        }
    }
    /*
    if(cp->ConnectServer()){
        cout << "Connect success" << endl;
    }
    */
}
