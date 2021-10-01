#include "socket.hpp"
#include "shell.hpp"
#include <iostream>
#include <cstring>
#include <thread>
using namespace std;
Server server;
int port = 8888;
inline int parse(char *str) {
	int len = strlen(str), x = 0;
	for(int i = 0; i < len; i++)
		x = x * 10 + str[i] ^ 48;
	return x;
}
inline void handle(Connection conn, Shell sh) {
	while(conn.good()) {
		string out;
		if(!sh.read(out))	continue;
		conn.send(out);
	}
}
int main(int argc, char **argv) {
	if(argc == 1) cout << "[!] No port specified! Use 8888 for default." << endl;
	else port = parse(argv[1]);
	cout << "[-] Initializing..." << endl;
	server.init();
	cout << "[!] xShell 0.0.2 started." << endl;
	cout << "[-] IP:" << getLocalIpAddress() << ':' << port << endl;
	server.listen(port);
	while(1) {
		auto conn = server.getConnection();
		cout << "[!] Incomming Connection." << endl;
		Shell sh;
		if(!sh.createProcess("cmd.exe"))    conn.send("ok");
		else {
			conn.send("failed");
			conn.terminate();
			cout << "[!] Disconnected." << endl;
			continue;
		}
		Sleep(100);
		if(!conn.good()) {
			sh.terminateProcess();
			cout << "[!] Disconnected." << endl;
			continue;
		}
		string cmd;
		thread t(handle, conn, sh);
		pthread_t ptr = t.native_handle();
		t.detach();
		while(conn.good()) {
			cmd = conn.receive();
			if(cmd == "exit")	break;
			sh.write(cmd);
		}
		pthread_cancel(ptr);
		sh.terminateProcess();
		cout << "[!] Disconnected." << endl;
	}
	server.destruct();
	system("pause");
	return 0;
}