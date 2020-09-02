#include "ServerSocket.h"

ServerSocket::ServerSocket() {}

ServerSocket::~ServerSocket() {}

bool ServerSocket::startServer() {
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        // WSAStartup failed with error, check iResult for more details
        return false;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    return true;
}

bool ServerSocket::resolveServer(const char* port) {
    // Resolve the server address and port
    iResult = getaddrinfo(NULL, port, &hints, &result);
    if (iResult != 0) {
        // getaddrinfo failed with error, check iResult for more details
        WSACleanup();
        return false;
    }
    return true;
}

bool ServerSocket::createListeningSocket() {
    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        // socket failed with error, use WSAGetLastError() for more details
        freeaddrinfo(result);
        WSACleanup();
        return false;
    }
    return true;
}

bool ServerSocket::setupListeningSocket() {
    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        // bind failed with error, use WSAGetLastError() for more details
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return false;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        // listen failed with error, use WSAGetLastError() for more details
        closesocket(ListenSocket);
        WSACleanup();
        return false;
    }

    return true;
}

bool ServerSocket::acceptClient() {
    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        // accept failed with error, use WSAGetLastError() for more details
        closesocket(ListenSocket);
        WSACleanup();
        return false;
    }
    return true;
}

bool ServerSocket::closeListeningSocket() {
    // No longer need server socket
    closesocket(ListenSocket);

    // Receive until the peer shuts down the connection
    do {

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            

            // Echo the buffer back to the sender
            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR) {
                // send failed with error, use WSAGetLastError() for more details
                closesocket(ClientSocket);
                WSACleanup();
                return false;
            }
        }
        
        else if (iResult < 0){
            // recv failed with error, use WSAGetLastError() for more details
            closesocket(ClientSocket);
            WSACleanup();
            return false;
        }

    } while (iResult > 0);

    return true;
}

bool ServerSocket::shutdown() {
    // shutdown the connection since we're done
    iResult = ::shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        // shutdown failed with error, use WSAGetLastError() for more details
        closesocket(ClientSocket);
        WSACleanup();
        return false;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return true;
}