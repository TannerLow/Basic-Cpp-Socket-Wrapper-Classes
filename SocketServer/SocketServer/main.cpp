#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include "ServerSocket.h"

#include <iostream>
#include <string>

int main(){
	ServerSocket server;
	server.startServer();
	server.resolveServer();
	server.createListeningSocket();
	server.setupListeningSocket();
	server.acceptClient();
	
	std::cout << server.receive().recvbuf << std::endl;
	std::string input;
	std::getline(std::cin, input);
	server.send(input.c_str(), true);

	server.closeListeningSocket();
	server.shutdown();
}