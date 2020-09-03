#pragma once

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

enum class Status { New, Started, Waiting, Ready, Listening, Closed, Shutdown };

struct RecvBuffer {
    char* recvbuf = nullptr;
    int   recvbuflen = -1;
};

class ServerSocket
{
private:
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    Status status;

public:
    ServerSocket();
    ~ServerSocket();

    bool startServer();
    bool resolveServer(const char* port = DEFAULT_PORT);
    bool createListeningSocket();
    bool setupListeningSocket();
    bool acceptClient();
    bool send(const char* sendbuf, const bool nullTerminated = false);
    RecvBuffer receive();
    bool closeListeningSocket();
    bool shutdown();
};

