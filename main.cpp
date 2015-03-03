#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "AppLayer.h"

using namespace std;


int main (int argc, char** argv){

	ifstream myReader;
	char* fileName = argv[1];
	myReader.open(fileName);
	cout << "The file name is " <<  fileName << endl;
	string line = "";
	int count = 0;
	while(getline(myReader,line)) {
		char *pch;
		char *linecopy = new char[line.length() + 1];
		strcpy(linecopy,line.c_str());
		pch = strtok(linecopy, ": ");

		while(pch != NULL) {
			cout << pch << endl;
			pch = strtok(NULL, " : ");
		}
	}
	string input = "";
	AppLayer myApp;
	while(1){
		cin >> input;
		myApp.runningApp(input);
	}

	return 0;
}
/*
IPLayer createIPLayer(string config) {
	char *pch;
	char *linecopy = new char[config.length() + 1];
	strcpy(linecopy, config.c_str());
	pch = strtok(linecopy, ": ");

	while(pch != NULL) {
		newPS.ip = inet_addr(pch);
		pch = strtok(NULL, " : ");
	}

	return 0;
}
*/
