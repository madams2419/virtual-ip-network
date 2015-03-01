#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>

using namespace std;

struct phySocket {
	sockaddr_in ip;
	int port;
};

struct routerPort {
	phySocket ps;
	sockaddr_in vipLoc;
	sockaddr_in vipRem;
};

phySocket parsePhySocket(string config);
sockaddr_in stringToIP(string sip);

int main (int argc, char** argv){

	phySocket myPhySocket;

	ifstream myReader;
	char* fileName = argv[1];
	myReader.open(fileName);

	string line = "";
	int count = 0;
	while(getline(myReader,line)){
		if(count == 0) {
			myPhySocket = parsePhySocket(line);
			continue;
		}

		char *pch;
		char *linecopy = new char[line.length() + 1];
		strcpy(linecopy,line.c_str());
		pch = strtok(linecopy, ": ");

		while(pch != NULL) {
			if(count == 0) {
			pch = strtok(NULL, " : ");
		}

		count++;
	}

	return 0;


}

phySocket parsePhySocket(string config) {
	struct phySocket newPS;

	char *pch;
	char *linecopy = new char[config.length() + 1];
	strcpy(linecopy, config.c_str());
	pch = strtok(linecopy, ": ");

	while(pch != NULL) {
		newPS.ip = inet_addr(pch);
		pch = strtok(NULL, " : ");
	}

	return 0;
}

sockaddr_in stringToIP(string sip) {
	// TODO
}
