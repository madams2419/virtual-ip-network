#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

#define MAX_ROUTES 128
#define MAX_TTL 64
#define MAX_HOST 128
#define DEFAULT_MTU 1400
#define MAX_RCV_LEN 64000

typedef struct {
	char* ipAddr;
	char* port;
} phy_info;

typedef struct {
	char* locAddr;
	char* rmtAddr;
	phy_info rmtPhy;
	int mtu;
} itf_info;

typedef struct {
	char* dest;
	char* nextHop;
	int cost;
	int TTL;
} route_entry;

#endif
