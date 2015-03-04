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

class IPLayer {

	private:
		std::map<u_int32_t, route_entry> routingTable;
		std::map<u_int32_t, int> fwdTable;
		std::queue<std::string> rcvQueue;
		std::vector<itf_info>* interfaces;
		pthread_rwlock_t rtLock, ftLock, rqLock;
		LinkLayer* linkLayer;
		int defaultItf;
		clock_t startTime;

		void runListening();
		void runRouting();
		int getFwdInterface(u_int32_t daddr);
		void handleNewPacket(char* packet, int len);
		struct iphdr* parseHeader(char* packet);
		void decrementTTL(char* packet);
		void deliverLocal(char* packet);
		void genHeader(char* buf, int dataLen, u_int32_t saddr, u_int32_t daddr, bool rip);
		static void *runThread(void* pkg);
		void bufSerialize(char* buf, int len);
		void printHeader(char* packet);
		void forward(char* packet, int itf);
		void sendRIPUpdates();
		int send(char* data, int dataLen, char* destIP, bool rip);
		void handleRIPPacket(char* packet);

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
};

#endif
