#ifndef LINKLAYER_H
#define LINKLAYER_H

#include <vector>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "constants.h"

class LinkLayer {

	private:
		phy_info localPhy;
		std::vector<itf_info> itfs;
		struct addrinfo* localAI;
		int rcvSocket;
		std::vector<int> sendSockets;
		std::vector< std::vector<char> > packetQueue;
		void start();
		int createSocket(phy_info phyInfo, struct addrinfo* ai, bool bindSock);

	public:
		LinkLayer(phy_info localPhy, std::vector<itf_info> itfs);
		int send(char* data, int dataLen, int itfNum);
		int listen(char* buf, int bufLen);
		char* getInterfaceAddr(int itf);
};

#endif

