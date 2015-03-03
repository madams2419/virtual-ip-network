#ifndef IPLAYER_H
#define IPLAYER_H

#include <vector>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "constants.h"


using namespace std;

class LinkLayer {

	private:
		phy_info localPhy;
		vector<itf_info> itfs;
		struct addrinfo* localAI;
		int rcvSocket;
		vector<int> sendSockets;
		vector< vector<char> > packetQueue;
		void start();
		int createSocket(phy_info phyInfo, struct addrinfo* ai, bool bindSock);

	public:
		LinkLayer(phy_info localPhy, vector<itf_info> itfs);
		int send(char* data, int dataLen, int itfNum);
		int listen(char* buf, int bufLen);
};

#endif

