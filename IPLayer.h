#ifndef IPLAYER_H
#define IPLAYER_H

#include <map>
#include <vector>
#include <queue>
#include <string>
#include "constants.h"
#include "LinkLayer.h"

#define MAX_MSG_LEN (512)
#define HDR_SIZE sizeof(struct iphdr)


class IPLayer {

	private:
		std::map<u_int32_t, route_entry> routingTable;
		std::map<u_int32_t, int> fwdTable;
		std::vector<char*> myAddreses;
		std::queue<std::string> rcvQueue;
		LinkLayer* linkLayer;
		int defaultItf;

		void runForwarding();
		void runRouting();
		int getFwdInterface(u_int32_t daddr);
		void handleNewPacket(char* packet, int len);
		struct iphdr* parseHeader(char* packet);
		void decrementTTL(char* packet);
		void deliverLocal(char* packet);
		struct iphdr* genHeader(int dataLen, u_int32_t saddr, u_int32_t daddr);
		static void *runThread(void* pkg);

		//DEBUG
		void popFwdTable();

	public:
		IPLayer(LinkLayer* linkLayer);
		int send(char* data, int dataLen, char* destIP);
		int receive(char* buf, int bufLen);
		bool hasData();
		std::string getData();
};

#endif
