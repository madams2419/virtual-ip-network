#include <vector>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include "constants.h"
#include "AppLayer.h"

using namespace std;

AppLayer::AppLayer(IPLayer* ipLayer) {
	this->ipLayer = ipLayer;
}

/**
 * Spawn worker thread to listen for messages on IP layer
 * Direct main thread to handle user input commands
 */
void AppLayer::run() {
	// spawn worker thread to listen for messages
	pthread_t* rcvWorker = new pthread_t;
	if (pthread_create(rcvWorker, NULL, runListener, (void*) this) != 0) {
		perror("Threading error.");
	}

	// direct main thread to handle commands
	getUserCommands();
}

/**
 * Static listener thread dispatch point
 */
void *AppLayer::runListener(void* apl) {
	AppLayer* app = (AppLayer*) apl;
	app->getIPData();
	return NULL;
}

/**
 * Continuously listen on IP layer and print and data received
 */
void AppLayer::getIPData() {
	while (1) {
		if (ipLayer->hasData()) {
			cout << "Message received" << endl;
			cout << "================================" << endl;
			cout << ipLayer->getData() << endl;
			cout << "================================" << endl;
			cout << "Please enter a command: " << endl;
		}
	}
}

/**
 * Listen on standard in and process user commands
 */
void AppLayer::getUserCommands() {
	string input = "";
	while(1){
		cout << "Please enter a command: " << endl;
		getline(cin, input);
		processCommand(input);
	}
}

/**
 * Send data via IPLayer
 */
void AppLayer::sendData(string instruction[], int counter){
	string message = "";
	for(int i = 2; i < counter; i++){
		message = message + instruction[i] + " ";
	}
	cout << "The destination is " << instruction[1] << " and the message is " << message << endl;

	ipLayer->send(const_cast<char* >(message.c_str()), message.length(), const_cast<char* >(instruction[1].c_str()));
}

/**
 * Parse and execute string command
 */
void AppLayer::processCommand(const string& command){
	string toDo[128];
	int count = 0;
	char* parse;
	char* line = new char[command.length()+1];
	strcpy(line, command.c_str());
	parse = strtok(line, "  ");

	cout << "The received message is: ";
	while(parse != NULL){
		string str = parse;
		toDo[count] = str;
		count++;
		cout << str << " ";
		parse = strtok(NULL, "  ");
	}
	cout << endl;

	string commandType = toDo[0];

	if(commandType.compare("send") == 0){
		cout << "The command is " << commandType << endl;
		sendData(toDo, count);
	} else if (commandType.compare("ipconfig") == 0) {
		cout << "The command is " << commandType << endl;
	} else if (commandType.compare("routes") == 0) {
		cout << "The command is " << commandType << endl;
	} else if (commandType.compare("up") == 0) {
		cout << "The command is " << commandType << endl;
	} else if (commandType.compare("down") == 0) {
		cout << "The command is " << commandType << endl;
	} else {
		cout << "The command is " << commandType << " which cannot be recognized. Please re-enter" << endl;
	}
}
