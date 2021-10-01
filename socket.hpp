#pragma once
#pragma comment (lib, "ws2_32.lib")

#include <winsock2.h>
#include <cstdlib>
#include <string>
#include <vector>

std::string getLocalIpAddress() {
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	if(WSAStartup(wVersionRequested, &wsaData) != 0)
		return "";
	char local[255] = {0};
	gethostname(local, sizeof(local));
	hostent *ph = gethostbyname(local);
	if(ph == NULL)
		return "";
	in_addr addr;
	memcpy(&addr, ph->h_addr_list[0], sizeof(in_addr));
	std::string localIP;
	localIP.assign(inet_ntoa(addr));
	WSACleanup();
	return localIP;
}

struct Connection {
	SOCKET sock;
	bool terminated;
	Connection(SOCKET s, int err) {
		terminated = false;
		sock = s;
		if(err != 0)  terminate();
	}
	bool good() {
		return !terminated;
	}
	void send(std::string message) {
		unsigned int len = htonl(message.size());
		if(::send(sock, (const char *)&len, sizeof(unsigned int), 0) == -1) {
			terminate();
			return;
		}
		if(::send(sock, message.c_str(), message.size(), 0) == -1)  terminate();
	}
	std::string receive() {
		unsigned int len;
		if(recv(sock, (char *)&len, sizeof(unsigned int), 0) == -1) {
			terminate();
			return "";
		}
		len = ntohl(len);
		std::vector<char> buf;
		buf.resize(len);
		if(recv(sock, &buf[0], len, 0) == -1) {
			terminate();
			return "";
		}
		std::string res;
		res.insert(res.begin(), buf.begin(), buf.end());
		return res;
	}
	void terminate() {
		closesocket(sock);
		terminated = true;
	}
};

class Client {
		WSAData wsaData;
		SOCKET sock;
		sockaddr_in serverAddr;
	public:
		void init() {
			WSAStartup(MAKEWORD(2, 2), &wsaData);
			sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
			memset(&serverAddr, 0, sizeof(serverAddr));
		}
		Connection connect(std::string ip, int port) {
			serverAddr.sin_family = PF_INET;
			serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
			serverAddr.sin_port = htons(port);
			int err = ::connect(sock, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
			return Connection(sock, err);
		}
		void destruct() {
			closesocket(sock);
			WSACleanup();
		}
};

class Server {
		WSAData wsaData;
		SOCKET sock;
		sockaddr_in sockAddr;
	public:
		void init() {
			WSAStartup(MAKEWORD(2, 2), &wsaData);
			sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
			memset(&sockAddr, 0, sizeof(sockAddr));
		}
		void listen(int port) {
			sockAddr.sin_family = PF_INET;
			sockAddr.sin_addr.s_addr = inet_addr(getLocalIpAddress().c_str());
			sockAddr.sin_port = htons(port);
			bind(sock, (SOCKADDR *)&sockAddr, sizeof(SOCKADDR));
			::listen(sock, 20);
		}
		Connection getConnection() {
			SOCKADDR clientAddr;
			int nSize = sizeof(SOCKADDR);
			return Connection(accept(sock, (SOCKADDR *)&clientAddr, &nSize), 0);
		}
		void destruct() {
			closesocket(sock);
			WSACleanup();
		}
};