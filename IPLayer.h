#ifndef IPLAYER_H
#define IPLAYER_H

#include <map>
#include <vector>
#include <queue>
#include <string>
#include <ctime>
#include <pthread.h>
#include "constants.h"
#include "LinkLayer.h"

#define HDR_SIZE sizeof(struct iphdr)

void bufSerialize(char* buf, int len);
u_int32_t inet_addr_h(char* addrStr);
std::string inet_htoa(u_int32_t addrInt);

class IPLayer {

	private:
		std::map<u_int32_t, route_entry> routingTable;
		std::queue<std::string> rcvQueue;
		std::vector<itf_info>* interfaces;
		pthread_rwlock_t rtLock, rqLock;
		LinkLayer* linkLayer;
		int defaultItf;
		bool debug;

		void runListening();
		void runRouting();
		int getFwdInterface(u_int32_t daddr);
		void handleNewPacket(char* packet, int len);
		struct iphdr* parseHeader(char* packet);
		void decrementTTL(char* packet);
		void deliverLocal(char* packet);
		void genHeader(char* buf, int dataLen, u_int32_t saddr, u_int32_t daddr, bool rip);
		static void *runThread(void* pkg);
		void printHeader(char* packet);
		void forward(char* packet, int itf);
		void broadcastRIPUpdates();
		int send(char* data, int dataLen, char* destIP, bool rip);
		void handleRIPPacket(char* packet);
		void initRoutingTable();
		void broadcastRIPRequests();
		void sendRIPUpdate(int itfNum);
		void sendRIPRequest(int itfNum);
		void updateRoutingTable(char* rdata, u_int32_t saddr);
		void printRIPPacket(char* rdata, u_int32_t saddr);
		bool mergeRoute(route_entry newrt);
		void clearExpiredRoutes();
		bool clearExpiredRoute(u_int32_t dest);

		//DEBUG
		void popFwdTable();

	public:
		IPLayer(LinkLayer* linkLayer);
		int send(char* data, int dataLen, char* destIP);
		int receive(char* buf, int bufLen);
		bool hasData();
		std::string getData();
		void printInterfaces();
		void printRoutes();
		void activateInterface(int itf);
		void deactivateInterface(int itf);
		void setMTU(int itf, int newMTU);
		void toggleDebug();
};

#endif
