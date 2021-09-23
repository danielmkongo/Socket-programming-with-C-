#pragma once
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iostream>

constexpr size_t MAXDATALENGTH = 4096;

#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

class Socket {
private:
	SOCKET Listen_sock, Work;
	sockaddr_in address;
	sockaddr_in clientaddress;
	int addlen;
	bool status;
	uint16_t port;

private:
	void configure(const short& port) {
		this->address.sin_addr.s_addr = INADDR_ANY;
		this->address.sin_family = AF_INET;
		this->port = port;
		this->address.sin_port = htons(port);
	}

	void configure(const unsigned& port, const char* Address) {
		address.sin_addr.s_addr = inet_addr(Address);
		address.sin_family = AF_INET;
		address.sin_port = htons(port);
	}

	void operator=(const SOCKET& other) {
		this->Work = other;
	}
public:

	std::string getClientName() {
		char host[NI_MAXHOST];
		char service[NI_MAXSERV];
		if (!getnameinfo((sockaddr*)&this->clientaddress, sizeof(this->clientaddress),
			host, NI_MAXHOST, service, NI_MAXSERV, NULL))
		{

			return host;
		}

		inet_ntop(AF_INET, &this->clientaddress.sin_addr, host, NI_MAXHOST);
		return host;
	}

	void listen(const short& port) {
		this->configure(port);
		this->Listen_sock = socket(AF_INET, SOCK_STREAM, NULL);
		bind(this->Listen_sock, (sockaddr*)&this->address, sizeof(this->address));
		::listen(this->Listen_sock, SOMAXCONN);
	}

	void listenAsync() {
		int result=::listen(this->Listen_sock, SOMAXCONN);
	}

	uint16_t getPort() {
		return this->port;
	}

	void listen() {
		this->configure(this->port);
		this->Listen_sock = socket(AF_INET, SOCK_STREAM, NULL);
		bind(this->Listen_sock, (sockaddr*)&this->address, sizeof(this->address));
		::listen(Listen_sock, SOMAXCONN);
	}

	bool accept(Socket& client) {
		client = ::accept(this->Listen_sock, (sockaddr*)&this->clientaddress, &addlen);
		if (client.getSocket() == 0) {
			throw "error could not establish connection!";
			return false;
		}
		else {
			return true;
		}
	}


	SOCKET accept() {
		SOCKET client = ::accept(this->Listen_sock, (sockaddr*)&this->clientaddress, &addlen);
		if (client == 0) {
			throw "error could not establish connection!";
			return 0;
		}
		else {
			return client;
		}
	}

	bool send(const char* data, const size_t& size) {
		if (::send(this->Work, data, (int)size, NULL)) {
			return true;
		}
		return false;
	}

	bool send(const char* data) {
		if (::send(this->Work, data, sizeof(data), NULL)) {
			return true;
		}
		return false;
	}

	bool isConnected() {
		return status;
	}

	bool connect(const char* ipaddress, const unsigned& port) {
		this->configure(port, ipaddress);
		if (::connect(this->Work, (sockaddr*)&this->address, this->addlen) != 0) {
			return false;
		}
		return true;
	}

	bool receive(char* buffer, const size_t& size, size_t& bytes) {
		bytes = recv(this->Work, buffer,(int)size, NULL);
		
		if (bytes > 0 && strlen(buffer) != NULL) {
			return true;
		}

		else if (bytes == 0) {
			std::cout << "\n" << this->getClientName() << "has disconnected!\n";
			this->status = false;
			return false;
		}
		return false;
	}

	void setConnectionStatus(bool status) {
		this->status = status;
	}

	SOCKET getSocket() {
		return this->Work;
	}
	bool setBlocking(bool blocking) {
		if (this->Work < 0) return false;

#ifdef _WIN32
		unsigned long mode = blocking ? 0 : 1;
		return (ioctlsocket(this->Work, FIONBIO, &mode) == 0) ? true : false;
#else
		int flags = fcntl(fd, F_GETFL, 0);
		if (flags == -1) return false;
		flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
		return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif
	}

	void close() {
		closesocket(this->Work);
		this->setConnectionStatus(false);
		WSACleanup();
	}


	Socket() {
		WSAData data;
		if (WSAStartup(MAKEWORD(2, 0), &data) != 0) {
			std::cerr << "error initiating...!";
		}
		memset(this, NULL, sizeof(*this));
		this->Work = socket(AF_INET, SOCK_STREAM, NULL);
		this->addlen = sizeof(address);
		this->status = false;
	}

	~Socket() {};

};