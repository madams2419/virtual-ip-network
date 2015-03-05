#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <ctime>

#include "LinkLayer.h"
#include "constants.h"
#include "ipsum.h"
#include "IPLayer.h"

#include "ip.h"
//#include <netinet/ip.h>

using namespace std;

typedef struct {
	IPLayer* ipl;
	string toRun;
} thread_pkg;

IPLayer::IPLayer(LinkLayer* link) {
	linkLayer = link;
	interfaces = linkLayer->getInterfaces();
	defaultItf = 0;
	startTime = clock();

	// initialize read write locks
	pthread_rwlock_init(&rtLock, NULL); // routing table lock
	pthread_rwlock_init(&rqLock, NULL); // request queue lock

	// initialize routing table
	initRoutingTable();

	// DEBUG
	printRoutes();

	// send requests on all intefaces
	sendRIPRequests();

	// create thread to listen for packets
	pthread_t* listWorker = new pthread_t;
	thread_pkg* lpkg = new thread_pkg;
	lpkg->ipl = this;
	lpkg->toRun = "listening";
	if(pthread_create(listWorker, NULL, &runThread, (void*) lpkg) != 0) {
		perror("Threading error:");
	}

	// create thread to send RIP updates
	pthread_t* ripWorker = new pthread_t;
	thread_pkg* rpkg = new thread_pkg;
	rpkg->ipl = this;
	rpkg->toRun = "rip_updating";
	if(pthread_create(ripWorker, NULL, &runThread, (void*) rpkg) != 0) {
		perror("Threading error:");
	}

}

/**
 * Initialize routing table based on interfaces
 */
void IPLayer::initRoutingTable() {
	pthread_rwlock_wrlock(&rtLock);
	for (int i = 0; i < interfaces->size(); i++) {
		itf_info itf = interfaces->at(i);
		u_int32_t dest = inet_addr(itf.rmtAddr);
		route_entry rentry;

		// populate route entry
		rentry.dest = dest;
		rentry.nextHop = i; // current interface number
		rentry.cost = 1; // one for adjacent nodes
		rentry.lastUpdate = clock(); // entry never expires

		routingTable[dest] = rentry;
	}
	pthread_rwlock_unlock(&rtLock);
}

/**
 * Print interfaces
 */
void IPLayer::printInterfaces() {
	linkLayer->printInterfaces();
}

/**
 * Print routes
 */
void IPLayer::printRoutes() {
	pthread_rwlock_rdlock(&rtLock);
	map<u_int32_t, route_entry>::iterator it = routingTable.begin();
	while(it != routingTable.end()) {
		u_int32_t dest = it->first;
		route_entry r = it->second;
		struct in_addr destIA;
		destIA.s_addr = dest;
		string destStr = inet_ntoa(destIA);
		cout << destStr << "\t" << r.itf << "\t" << r.cost << endl;
		it++;
	}
	pthread_rwlock_unlock(&rtLock);
}

/**
 * Activate interface
 */
void IPLayer::activateInterface(int itf) {
	if (itf >= interfaces->size()) {
		printf("Interface %d not found.\n", itf + 1);
	} else {
		interfaces->at(itf).down = false;
		printf("Interface %d up.\n", itf + 1);
	}
}

/**
 * Deactivate interface
 */
void IPLayer::deactivateInterface(int itf) {
	if (itf >= interfaces->size()) {
		printf("Interface %d not found.\n", itf + 1);
	} else {
		interfaces->at(itf).down = true;
		printf("Interface %d down.\n", itf + 1);
	}
}

/**
 * Static worker thread dispatch point
 */
void *IPLayer::runThread(void* pkg) {
	thread_pkg* tp = (thread_pkg*) pkg;
	IPLayer* ipl = tp->ipl;
	string toRun = tp->toRun;

	if(toRun.compare("listening"))
		ipl->runListening();
	else if (toRun.compare("rip_updating"))
		ipl->runRouting();
	else
		cout << "Invalid toRun command." << endl;
}

/**
 * Loops continuously listening for packets
 * Dispatches a new worker thread to handle packets when received
 */
void IPLayer::runListening() {
	int rcvLen;
	char buf[MAX_RCV_LEN];

	while (1) {
		// get packet
		rcvLen = linkLayer->listen(buf, sizeof(buf));
		if (rcvLen < 0) {
			printf("IP Layer receive error.\n");
			continue;
		} else {
			printf("IP packet received.\n");
		}

		// copy received packet into appropriately sized buffer
		char packet[rcvLen];
		memcpy(packet, buf, sizeof(packet));

		//TODO spawn new thread here
		handleNewPacket(packet, sizeof(packet));

		// zero out receive buffer
		memset(buf, 0, sizeof(buf));
	}
}

/**
 * Loops continuously sending RIP updates at regular intervals on all active interfaces
 */
void IPLayer::runRouting() {
	clock_t start = clock();
	while (1) {
		clock_t end  = clock();
		double split = (end - start) / (double) CLOCKS_PER_SEC;
		if (split >= RIP_UPDATE_INT) {
			// send RIP updates
			sendRIPUpdates();
			// reset start clock
			start = clock();
		}
	}
}

/**
 * Sends RIP update to all active interfaces
 */
void IPLayer::sendRIPUpdates() {
	for (int j = 0; j < interfaces->size(); j++) {
		itf_info itf = interfaces->at(j);

		// continue if interface is down
		if (itf.down) continue;

		// populate array of rip route entries
		int numRoutes = routingTable.size();
		pthread_rwlock_rdlock(&rtLock);
		rip_entry routes[numRoutes];
		map<u_int32_t, route_entry>::iterator it = routingTable.begin();
		int cnt = 0;
		while(it != routingTable.end()) {
			u_int32_t dest = it->first;
			route_entry rentry = it->second;

			routes[cnt].cost = rentry.cost;
			routes[cnt].address = dest;

			cnt++;
			it++;
		}
		pthread_rwlock_unlock(&rtLock);

		// format RIP update data
		int routesSize = numRoutes * sizeof(rip_entry);

		// DEBUG
		cout << "routesSize : " << routesSize << endl;
		cout << "sizeof(routes) : " << sizeof(routes) << endl;

		char packet[sizeof(rip_hdr) + routesSize];
		rip_hdr* rip = (rip_hdr*) packet;

		// form RIP header
		rip->command = 2; // response command
		rip->num_entries = numRoutes; // size of routing table

		// copy rip route entry array
		memcpy(&packet[sizeof(rip_hdr)], (char*) routes, routesSize);

		// send RIP packet
		send(packet, sizeof(packet), itf.rmtAddr, true);
	}
}

/**
 * Sends RIP update to all active interfaces
 */
void IPLayer::sendRIPRequests() {
	for (int j = 0; j < interfaces->size(); j++) {
		itf_info itf = interfaces->at(j);

		// continue if interface is down
		if (itf.down) continue;

		// format RIP update data
		char packet[sizeof(rip_hdr)];
		rip_hdr* rip = (rip_hdr*) packet;

		// form RIP header
		rip->command = 1; // request command
		rip->num_entries = 0; // no routes in request command

		// send RIP packet
		send(packet, sizeof(packet), itf.rmtAddr, true);
	}
}

/**
 * Handles new packets by forwarding or delivering them locally
 */
void IPLayer::handleNewPacket(char* packet, int len) {
	int fwdItf;
	int checksum;
	struct iphdr* hdr;

	// convert packet to host byte order
	bufSerialize(packet, len);

	// parse header
	hdr = (struct iphdr*) packet;

	//DEBUG
	cout << "Received packet..." << endl;
	printHeader(packet);

	// check to make sure receive length equals header total length
	if (hdr->tot_len != len) {
		printf("Partial packet received, discarding.\n");
		return;
	}

	// verify checksum
	u_int16_t rcvCheck = hdr->check;
	hdr->check = 0;
	if (rcvCheck != ip_sum(packet, HDR_SIZE)) {
		printf("Invalid checksum, discarding packet.\n");
		return;
	}

	// forward, delivery locally, or handle RIP
	if ((fwdItf = getFwdInterface(hdr->daddr)) == -1) {
		if (hdr->protocol == 200)
			handleRIPPacket(packet);
		else
			deliverLocal(packet);
	} else {
		forward(packet, fwdItf);
	}

}

/**
 * Processes RIP packet
 */
void IPLayer::handleRIPPacket(char* packet) {
	cout << "GOT RIP PACKET!!!" << endl;
}

/**
 * Forwards packet on specified interface. Packet must be host order.
 */
void IPLayer::forward(char* packet, int itf) {
	struct iphdr* hdr = (struct iphdr*) packet;
	int len = hdr->tot_len;

	// decrement ttl or return if zero
	if (hdr->ttl == 0) {
		printf("TTL = 0, discarding packet.\n");
		return;
	} else { // decrement ttl and recalculate checksum
		hdr->ttl--;
		hdr->check = 0;
		hdr->check = ip_sum(packet, HDR_SIZE);
	}

	// convert packet to network byte order
	bufSerialize(packet, len);

	// send packet via link layer
	int bytesSent;
	if((bytesSent = linkLayer->send(packet, len, itf)) < 0) {
		printf("Sending error.\n");
	} else {
		printf("Forwarded %d bytes on interface %d.\n", bytesSent, itf);
	}
}

/**
 * Returns the next string in the receive buffer
 */
string IPLayer::getData() {
	pthread_rwlock_wrlock(&rqLock);
	string data = rcvQueue.front();
	rcvQueue.pop();
	pthread_rwlock_unlock(&rqLock);
	return data;
}

/**
 * Return true if the IP layer has buffered data
 */
bool IPLayer::hasData() {
	pthread_rwlock_rdlock(&rqLock);
	bool hasData = (rcvQueue.size() > 0);
	pthread_rwlock_unlock(&rqLock);
	return hasData;
}

/**
 * Copies packet data into receive queue and makes it available for retreival by the application layer
 */
void IPLayer::deliverLocal(char* packet) {
	struct iphdr* hdr = (struct iphdr*) packet;

	// compute data length
	int hswop = hdr->ihl * 4; // header size with options
	int dataLen = hdr->tot_len - hswop;

	// copy packet data into string
	string data(&packet[hswop], dataLen);

	// add data buffer to data vector
	pthread_rwlock_wrlock(&rqLock);
	rcvQueue.push(data);
	pthread_rwlock_unlock(&rqLock);
}

/**
 * Send generic data (public send function)
 */
int IPLayer::send(char* data, int dataLen, char* destIP) {
	send(data, dataLen, destIP, false);
}

/**
 * Encapsulates RIP packet or generic data in IP header and sends via link layer
 */
int IPLayer::send(char* data, int dataLen, char* destIP, bool rip) {
	int bytesSent, itfNum;
	u_int32_t daddr, saddr;
	struct iphdr* hdr;

	// convert destination ip in dots-and-number form to network order int form
	daddr = inet_addr(destIP);

	// get LinkLayer interface to send packet over
	if ((itfNum = getFwdInterface(daddr)) < 0) {
		printf("Source address belongs to host. Delivering locally.\n");
		string locData(data, dataLen);
		pthread_rwlock_wrlock(&rqLock);
		rcvQueue.push(locData);
		pthread_rwlock_unlock(&rqLock);
		return 0;
	}

	// fragment if data length is greater than interface mtu
	int packetLen = dataLen + HDR_SIZE;
	int mtu = linkLayer->getMTU(itfNum);
	if (packetLen > mtu) {
		printf("Packet length greater than interface MTU. Fragmenting...");
		//TODO fragmentation
		return -1;
	}

	// initialize buffer to store new packet
	char packet[packetLen];

	// get local IP address associated with interface in network order int form
	saddr = inet_addr(linkLayer->getInterfaceAddr(itfNum));

	// generate new data IP header
	genHeader(packet, dataLen, saddr, daddr, rip);

	// copy data to packet buffer
	memcpy(&packet[HDR_SIZE], data, dataLen);

	//DEBUG
	cout << "Sending packet..." << endl;
	printHeader(packet);

	// convert packet buffer to network byte order
	bufSerialize(packet, packetLen);

	// send packet via link layer
	if((bytesSent = linkLayer->send(packet, packetLen, itfNum)) < 0) {
		printf("Sending error.\n");
		return -1;
	}

	return bytesSent;
}

/**
 * Inserts header at the beginning of the supplied buffer
 */
void IPLayer::genHeader(char* buf, int dataLen, u_int32_t saddr, u_int32_t daddr, bool rip) {
	struct iphdr* hdr = (struct iphdr*) buf;

	// pack header
	hdr->version = 4; // IP version 4
	hdr->ihl = 5; // no options
	hdr->tos = 0; // no TOS protocol
	hdr->tot_len = (hdr->ihl * 4) + dataLen; // header length (in bytes) + data length
	hdr->id = 0; // fragmentation not supported
	hdr->frag_off = 0; // fragmentation not supported
	hdr->ttl = MAX_TTL; // maximum TTL
	hdr->protocol = (rip) ? 200 : 0; // 200 for RIP, 0 for data
	hdr->check = 0; // checksum is zero for calculation
	hdr->saddr = saddr; // source address in network byte order
	hdr->daddr = daddr; // destination address in network byte order

	// calculate checksum
	hdr->check = ip_sum((char*) hdr, HDR_SIZE);
}

void IPLayer::printHeader(char* packet) {
	struct iphdr* hdr = (struct iphdr*) packet;

	cout << "IP Header" << endl;
	cout << "=================" << endl;
	cout << "ver: " << hdr-> version << endl;
	cout << "ihl: " << hdr->ihl << endl;
	cout << "tos: " << hdr->tos << endl;
	cout << "len: " << hdr->tot_len << endl;
	cout << "id:  " << hdr->id << endl;
	cout << "frg: " << hdr->frag_off << endl;
	cout << "ttl: " << hdr->ttl << endl;
	cout << "prt: " << hdr->protocol << endl;
	cout << "chk: " << hdr->check << endl;
	cout << "sad: " << hdr->saddr << endl;
	cout << "dad: " << hdr->daddr << endl;
	cout << "=================" << endl;
}

/**
 * Convert arbitrary length buffer from host to network order and visa versa
 */
void IPLayer::bufSerialize(char* buf, int len) {
	// return if host byte order equals network byte order
	if (__BYTE_ORDER == __BIG_ENDIAN) return;

	// copy buffer to temporary storage
	char temp[len];
	memcpy(temp, buf, len);

	for (int i = 0; i < len; i++) {
		buf[i] = temp[len - 1 - i];
	}

}

/**
 * Gets the interface number to use for forwarding the given IP address.
 * Returns -1 if the address matches one of the interface addresses.
 */
int IPLayer::getFwdInterface(u_int32_t daddr) {
	// check if network number is equal to the destination of any of the interfaces
	if (linkLayer->isLocalAddr(daddr)) {
		printf("Destination address is local address. Delivering locally.\n");
		return -1;
	}

	int ret;
	pthread_rwlock_rdlock(&rtLock);
	if (routingTable.count(daddr) == 1) { // daddr is in fwd table; return itf value
		printf("Fwd table entry found. Forwarding on itf: %d\n", routingTable[daddr].itf);
		ret = routingTable[daddr].itf;
	} else { // daddr is not in fwd table; return default itf
		printf("Fwd table entry not found. Forwarding on default port.\n");
		ret = defaultItf;
	}
	pthread_rwlock_unlock(&rtLock);

	return ret;
}
