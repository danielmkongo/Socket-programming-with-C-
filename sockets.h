#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

constexpr size_t MAXDATALENGTH = 4096;

class Socket {
private:
    SOCKET listenSock = INVALID_SOCKET;
    SOCKET workSock   = INVALID_SOCKET;
    sockaddr_in address{};
    sockaddr_in clientAddress{};
    int addrLen = sizeof(sockaddr_in);
    bool connected = false;
    uint16_t port = 0;

    void configureServer(uint16_t p) {
        port = p;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
    }

    void configureClient(uint16_t p, const char* ip) {
        port = p;
        address.sin_family = AF_INET;
        inet_pton(AF_INET, ip, &address.sin_addr);
        address.sin_port = htons(port);
    }

public:
    /* ===== INIT ===== */
    Socket() {
        static bool wsaInit = false;
        if (!wsaInit) {
            WSADATA data;
            if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
                throw std::runtime_error("WSAStartup failed");
            }
            wsaInit = true;
        }

        workSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (workSock == INVALID_SOCKET) {
            throw std::runtime_error("socket() failed");
        }
    }

    ~Socket() {
        close();
    }

    /* ===== SERVER ===== */
    bool listen(uint16_t p) {
        configureServer(p);

        listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSock == INVALID_SOCKET) return false;

        if (bind(listenSock, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
            return false;

        if (::listen(listenSock, SOMAXCONN) == SOCKET_ERROR)
            return false;

        return true;
    }

    bool accept(Socket& client) {
        SOCKET s = ::accept(listenSock, (sockaddr*)&clientAddress, &addrLen);
        if (s == INVALID_SOCKET) return false;

        client.workSock = s;
        client.clientAddress = clientAddress;
        client.connected = true;
        return true;
    }

    /* ===== CLIENT ===== */
    bool connect(const char* ip, uint16_t p) {
        configureClient(p, ip);

        if (::connect(workSock, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
            return false;

        connected = true;
        return true;
    }

    /* ===== DATA ===== */
    bool send(const char* data, size_t size) {
        return (::send(workSock, data, (int)size, 0) > 0);
    }

    bool send(const char* data) {
        return send(data, strlen(data));
    }

    bool receive(char* buffer, size_t size, size_t& bytes) {
        bytes = recv(workSock, buffer, (int)size, 0);

        if (bytes > 0) {
            return true;
        }
        if (bytes == 0) {
            connected = false;
        }
        return false;
    }

    std::string getClientName() {
        char host[NI_MAXHOST];
        inet_ntop(AF_INET, &clientAddress.sin_addr, host, NI_MAXHOST);
        return host;
    }

    bool isConnected() const {
        return connected;
    }

    SOCKET getSocket() const {
        return workSock;
    }

    void close() {
        if (workSock != INVALID_SOCKET) {
            closesocket(workSock);
            workSock = INVALID_SOCKET;
        }
        connected = false;
    }

    bool setBlocking(bool blocking) {
        u_long mode = blocking ? 0 : 1;
        return ioctlsocket(workSock, FIONBIO, &mode) == 0;
    }
};
