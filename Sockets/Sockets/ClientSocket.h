#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

enum class Status { New, Started, Waiting, Connected, Shutdown };

struct RecvBuffer {
    char* recvbuf    = nullptr;
    int   recvbuflen = -1;
};

class ClientSocket
{
private:
	WSAData wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    Status status;

public:
    ClientSocket();
    ~ClientSocket();

    bool startClient();
    bool resolveServer(const char* ipAddr, const char* port = DEFAULT_PORT);
    bool connect();
    bool send(const char* sendbuf, const bool nullTerminated = false);
    RecvBuffer recieve();
    bool shutdown();

    Status getStatus() const;
};

