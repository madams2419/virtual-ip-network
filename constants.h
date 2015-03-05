#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
#include <stdint.h>
#include <ctime>

#define MAX_ROUTES 128
#define MAX_TTL 64
#define MAX_HOST 128
#define DEFAULT_MTU 1400
#define MAX_RCV_LEN 64000
#define RIP_UPDATE_INT 5
#define ROUTE_EXP_TIME 12

typedef struct {
	char* ipAddr;
	char* port;
} phy_info;

typedef struct {
	char* locAddr;
	char* rmtAddr;
	phy_info rmtPhy;
	int mtu;
	bool down;
} itf_info;

typedef struct {
	uint32_t dest;
	uint32_t nextHop;
	int cost;
	clock_t lastUpdate;
	int itf;
} route_entry;

typedef struct {
	uint32_t cost;
	uint32_t address;
} rip_entry;

typedef struct {
	uint16_t command;
	uint16_t num_entries;
} rip_hdr;


#endif
