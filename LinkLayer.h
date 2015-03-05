#ifndef LINKLAYER_H
#define LINKLAYER_H

#include <vector>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "constants.h"

class LinkLayer {

	private:
		phy_info localPhy;
		std::vector<itf_info> itfs;
		struct addrinfo* localAI;
		int rcvSocket;
		std::vector<pthread_mutex_t> ssLocks;
		pthread_mutex_t rsLock;

		void start();
		int createSocket(phy_info phyInfo, struct addrinfo* ai, bool bindSock);

	public:
		LinkLayer(phy_info localPhy, std::vector<itf_info> itfs);
		int send(char* data, int dataLen, int itfNum);
		int listen(char* buf, int bufLen);
		char* getInterfaceAddr(int itf);
		int getInterfaceID(u_int32_t addr);
		int getMTU(int itfNum) {return itfs[itfNum].mtu;};
		void setMTU(int itfNum, int mtu) {itfs[itfNum].mtu = mtu;};
		std::vector<itf_info>* getInterfaces() {return &itfs;};
		void printInterfaces();
};

#endif

