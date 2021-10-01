#include "socket.hpp"
#include <iostream>
#include <thread>
using namespace std;
Client client;
string ip, cmd;
inline void handle(Connection conn) {
	while(conn.good())  cout << conn.receive();
}
int main() {
	while(1) {
		cin.ignore();
		cout << "[-] Initializing..." << endl;
		client.init();
		cout << "[!] xShell 0.0.2 started." << endl;
		cout << "[-] Input ip:";
		cin >> ip;
		cout << "[-] Connecting...";
		auto conn = client.connect(ip, 8888);
		if(!conn.good())  cout << "Failed :(" << endl;
		else {
			cout << endl;
			cout << "[-] Initializing remote shell...";
			if(conn.receive() != "ok")    cout << "Failed :(" << endl;
			else {
				cout << "Done." << endl;
				thread t(handle, conn);
				pthread_t ptr = t.native_handle();
				t.detach();
				while(getline(cin, cmd) && cmd != "exit" && conn.good())
					conn.send(cmd);
				pthread_cancel(ptr);
			}
		}
		conn.terminate();
		client.destruct();
	}
	system("pause");
	return 0;
}