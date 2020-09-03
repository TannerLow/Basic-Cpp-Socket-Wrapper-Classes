#include "ServerSocket.h"

ServerSocket::ServerSocket() {
    status = Status::New;
}

ServerSocket::~ServerSocket() {}

bool ServerSocket::startServer() {
    if (status != Status::New) return false; // Called out of order

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

    status = Status::Started;
    return true;
}

bool ServerSocket::resolveServer(const char* port) {
    if (status != Status::Started) return false; // Called out of order

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, port, &hints, &result);
    if (iResult != 0) {
        // getaddrinfo failed with error, check iResult for more details
        WSACleanup();
        return false;
    }
    status = Status::Waiting;
    return true;
}

bool ServerSocket::createListeningSocket() {
    if (status != Status::Waiting) return false; // Called out of order

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        // socket failed with error, use WSAGetLastError() for more details
        freeaddrinfo(result);
        WSACleanup();
        return false;
    }

    status = Status::Ready;
    return true;
}

bool ServerSocket::setupListeningSocket() {
    if (status != Status::Ready) return false; // Called out of order

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

    status = Status::Listening;
    return true;
}

bool ServerSocket::acceptClient() {
    if (status != Status::Listening) return false; // Called out of order

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

bool ServerSocket::send(const char* sendbuf, const bool nullTerminated) {
    if (status != Status::Listening) return false;

    iSendResult = ::send(ClientSocket, sendbuf, (int) strlen(sendbuf) + nullTerminated, 0);
    if (iSendResult == SOCKET_ERROR) {
        // send failed with error, use WSAGetLastError() for more details
        shutdown();
        return false;
    }

    return true;
}

RecvBuffer ServerSocket::receive() {
    RecvBuffer rb;
    if (status != Status::Listening) return rb;

    iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0) 
        rb = { recvbuf, recvbuflen };
    else if (iResult < 0) 
        shutdown(); // recv failed with error, use WSAGetLastError() for more details
    else 
        status = Status::Closed;

    return rb;
}

bool ServerSocket::closeListeningSocket() {
    if (status != Status::Listening) return false; // Called out of order

    // No longer need server socket
    closesocket(ListenSocket);

    // Receive until the peer shuts down the connection
    do {

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult < 0){
            // recv failed with error, use WSAGetLastError() for more details
            shutdown();
            return false;
        }

    } while (iResult > 0);

    status = Status::Closed;
    return true;
}

bool ServerSocket::shutdown() {
    if (status != Status::Closed) return false; // Called out of order

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

    status = Status::Shutdown;
    return true;
}