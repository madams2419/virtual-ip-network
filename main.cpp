#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "AppLayer.h"
#include "LinkLayer.h"
#include "IPLayer.h"

using namespace std;

const string DEFAULT_IP = "127.0.0.1";

int main (int argc, char** argv){

	// parse input arguments
	ifstream myReader;
	char* fileName = argv[1];
	myReader.open(fileName);
	//cout << "The file name is " <<  fileName << endl;
	string line = "";
	string fileInfo[MAX_HOST];
	int count = 0;
	while(getline(myReader,line)) {
		char *pch;
		char *linecopy = new char[line.length() + 1];
		strcpy(linecopy,line.c_str());
		pch = strtok(linecopy, ": ");
		while(pch != NULL) {
			string str = pch;
			fileInfo[count] = str;
			pch = strtok(NULL, " : ");
			count++;
		}
	}

	// populate interface vector
	vector<itf_info> nodeItfs;
	for(int i = 2; i < count;){
		itf_info newItf;
		phy_info newPhy;
		newPhy.ipAddr = const_cast<char* >(fileInfo[i].c_str());
		i++;
		newPhy.port = const_cast<char* >(fileInfo[i].c_str());
		i++;
		newItf.rmtPhy = newPhy;
		newItf.locAddr = const_cast<char* >(fileInfo[i].c_str());
		i++;
		newItf.rmtAddr = const_cast<char* >(fileInfo[i].c_str());
		i++;
		newItf.mtu = DEFAULT_MTU;
		newItf.down = false;
		nodeItfs.push_back(newItf);
	}

	// populate local phy info struct
	phy_info myPhyInfo;
	myPhyInfo.ipAddr = const_cast<char* >(fileInfo[0].c_str());
	myPhyInfo.port = const_cast<char* >(fileInfo[1].c_str());

	// create link, IP, and application layers
	LinkLayer* linkLayer = new LinkLayer(myPhyInfo, nodeItfs);
	IPLayer* ipLayer = new IPLayer(linkLayer);
	AppLayer* myApp = new AppLayer(ipLayer);

	// run app layer
	myApp->run();

	return 0;
}
