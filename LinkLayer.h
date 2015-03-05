#ifndef LINKLAYER_H
#define LINKLAYER_H

#include <vector>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
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
		u_int32_t getInterfaceAddr(int itf, std::string type);
		int getInterfaceFromAddr(u_int32_t addr, std::string type);

	public:
		LinkLayer(phy_info localPhy, std::vector<itf_info> itfs);
		int send(char* data, int dataLen, int itfNum);
		int listen(char* buf, int bufLen);
		u_int32_t getInterfaceLocalAddr(int itf);
		u_int32_t getInterfaceRemoteAddr(int itf);
		int getInterfaceFromLocalAddr(u_int32_t addr);
		int getInterfaceFromRemoteAddr(u_int32_t addr);
		int getMTU(int itfNum) {return itfs[itfNum].mtu;};
		void setMTU(int itfNum, int mtu) {itfs[itfNum].mtu = mtu;};
		std::vector<itf_info>* getInterfaces() {return &itfs;};
		void printInterfaces();
		bool itfNumValid(int itfNum);
};

#endif

