#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

#define MAX_ROUTES 128
#define MAX_TTL 120

typedef struct {
	char* ipAddr;
	char* port;
} phy_info;

typedef struct {
	char* locAddr;
	char* rmtAddr;
	phy_info rmtPhy;
} itf_info;


typedef struct {
	char* dest;
	char* nextHop;
	int cost;
	int TTL;
} route_entry;

#endif
