#include "ClientSocket.h"

ClientSocket::ClientSocket() {
    status = Status::New;
}

ClientSocket::~ClientSocket() {}

bool ClientSocket::startClient() {
    if (status != Status::New) return false; // Called out of order

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (iResult == 0) {
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    }
    else {
        //WSAStartup failed with error, check iResult for more details
        return false;
    }
    status = Status::Started;
    return true;
}

bool ClientSocket::resolveServer(const char* ipAddr, const char* port) {
    if (status != Status::Started) return false; // Called out of order

    // Resolve the server address and port
    iResult = getaddrinfo(ipAddr, port, &hints, &result);
    if (iResult != 0) {
        //getaddrinfo failed with error, check iResult for more details
        WSACleanup();
        return false;
    }
    status = Status::Waiting;
    return true;
}

bool ClientSocket::connect() {
    if (status != Status::Waiting) return false; // Called out of order

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            // socket failed with error, use WSAGetLastError() for more details
            WSACleanup();
            return false;
        }

        // Connect to server.
        iResult = ::connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        // Unable to connect to server!
        WSACleanup();
        return false;
    }
    status = Status::Connected;
    return true;
}

bool ClientSocket::send(const char* sendbuf, const bool nullTerminated) {
    if (status != Status::Connected) return false; // Called out of order

    // Send an initial buffer
    iResult = ::send(ConnectSocket, sendbuf, (int)strlen(sendbuf) + nullTerminated, 0);
    if (iResult == SOCKET_ERROR) {
        // send failed with error, use WSAGetLastError() for more details
        shutdown();
        return false;
    }

    return true;
}

RecvBuffer ClientSocket::receive() {
    RecvBuffer rb;
    if (status != Status::Connected) return rb; // Called out of order

    iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0)
        rb = { recvbuf, recvbuflen };
    else if (iResult == 0)
        shutdown(); // Connection closed

    // if iResult < 0 then recv failed with error, use WSAGetLastError() for more details

    return rb;
}

bool ClientSocket::shutdown() {
    if (status != Status::Connected) return false; // Called out of order

    // shutdown the connection since no more data will be sent
    iResult = ::shutdown(ConnectSocket, SD_SEND);
    status = Status::Shutdown;
    if (iResult == SOCKET_ERROR) {
        // shutdown failed with error, use WSAGetLastError() for more details
        closesocket(ConnectSocket);
        WSACleanup();
        return false;
    }

    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        // if < 0 then recv failed with error, use WSAGetLastError() for more details

    } while (iResult > 0);

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
    return true;
}

Status ClientSocket::getStatus() const {
    return status;
}