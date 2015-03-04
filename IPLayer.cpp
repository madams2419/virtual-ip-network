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
	defaultItf = 0;

	//DEBUG
	popFwdTable();

	// create thread to handle forwarding tasks
	pthread_t* fwdWorker = new pthread_t;
	thread_pkg* pkg = new thread_pkg;
	pkg->ipl = this;
	pkg->toRun = "forwarding";
	if(pthread_create(fwdWorker, NULL, &runThread, (void*) pkg) != 0) {
		perror("Threading error:");
	}

}

/**
 * Static worker thread dispatch point
 */
void *IPLayer::runThread(void* pkg) {
	thread_pkg* tp = (thread_pkg*) pkg;
	IPLayer* ipl = tp->ipl;
	string toRun = tp->toRun;
	ipl->runForwarding();
}

/**
 * Loops continuously listening for packets
 * Dispatches a new worker thread to handle packets when received
 */
void IPLayer::runForwarding() {
	int rcvLen;
	char buf[MAX_RCV_LEN];

	while(1) {
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

void IPLayer::runRouting() {
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
	hdr = (struct iphdr*) packet; //TODO might want to deal with network ordering issues

	//DEBUG
	cout << "Received packet..." << endl;
	printHeader(packet);

	// check to make sure receive length equals header total length
	if (hdr->tot_len != len) {
		printf("Partial packet received, discarding.\n");
		return;
	}

	// verfiy checksum
	u_int16_t rcvCheck = hdr->check;
	hdr->check = 0;
	if (rcvCheck != ip_sum(packet, HDR_SIZE)) {
		printf("Invalid checksum, discarding packet.\n");
		return;
	}

	// decrement ttl or return if zero
	if (hdr->ttl == 0) {
		printf("TTL = 0, discarding packet.\n");
		return;
	} else { // decrement ttl and recalculate checksum
		hdr->ttl--;
		hdr->check = ip_sum(packet, HDR_SIZE);
	}

	// forward or deliver locally
	if ((fwdItf = getFwdInterface(hdr->daddr)) == -1) {
		deliverLocal(packet);
	} else {
		forward(packet, len, fwdItf);
	}

}

/**
 * Forwards packet on specified interface. Packet must be host order.
 */
void IPLayer::forward(char* packet, int len, int itf) {
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
	string data = rcvQueue.front();
	rcvQueue.pop();
	return data;
}

/**
 * Return true if the IP layer has buffered data
 */
bool IPLayer::hasData() {
	return (rcvQueue.size() > 0);
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
	rcvQueue.push(data);
}

/**
 * Encapsulates data in IP header and sends via link layer
 */
int IPLayer::send(char* data, int dataLen, char* destIP) {
	int bytesSent, itfNum;
	u_int32_t daddr, saddr;
	struct iphdr* hdr;

	// get LinkLayer interface to send packet over
	if ((itfNum = getFwdInterface(daddr)) < 0) {
		printf("Source address belongs to host. Aborting send.\n");
		return -1;
	}

	// fragement if data length is greater than interface mtu
	int packetLen = dataLen + HDR_SIZE;
	int mtu = linkLayer->getMTU(itfNum);
	if (packetLen > mtu) {
		printf("Packet length greater than interface MTU. Fragmenting...");
		//TODO fragmentation
		return -1;
	}

	// initialize buffer to store new packet
	char packet[packetLen];

	// convert destination ip in dots-and-number form to network order int form
	daddr = inet_addr(destIP);

	// get local IP address associated with interface in network order int form
	saddr = inet_addr(linkLayer->getInterfaceAddr(itfNum));

	// generate new data IP header
	genHeader(packet, dataLen, saddr, daddr, false);

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
	} if (fwdTable.count(daddr) == 1) { // daddr is in fwd table; return itf value
		printf("Fwd table entry found. Forwarding on itf: %d\n", fwdTable[daddr]);
		return fwdTable[daddr];
	} else { // daddr is not in fwd table; return default itf
		printf("Fwd table entry not found. Forwarding on default port.\n");
		return defaultItf;
	}

}

void IPLayer::popFwdTable() {
	fwdTable[inet_addr("10.116.89.157")] = 0;
	fwdTable[inet_addr("14.230.5.36")] = 1;
}
