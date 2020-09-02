#include "ClientSocket.h"

#include <iostream>
#include <string>

int main()
{
    ClientSocket client;
    client.startClient();
    client.resolveServer("localhost");
    client.connect();

    std::string input;
    std::getline(std::cin, input);
    client.send(input.c_str(), true);
    
    client.shutdown();
}
