#ifndef IPLAYER_H
#define IPLAYER_H

#include <vector>
#include <string>
#include <stdlib>
#include <sys/socket>
#include <arpa/inet.h>
#include <unistd>
#include <netdb>
#include "constants.h"


using namespace std;

class AppLayer {

	private:
		void start();

	public:
		AppLayer();

};

#endif
