//Allows strcat
#define _CRT_SECURE_NO_WARNINGS

#include "ClientSocket.h"

#include <iostream>
#include <string>

int main()
{
    ClientSocket client;
    client.startClient();
    client.resolveServer("localhost");
    client.connect();
    //send data to server
    std::string input;
    std::getline(std::cin, input);
    client.send(input.c_str(), true);
    //recieve data from server
    char* received = strcat(client.receive().recvbuf, "\0");
    std::cout << received << std::endl;
    
    client.shutdown();
}
