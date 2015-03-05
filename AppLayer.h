#ifndef APPLAYER_H
#define APPLAYER_H

#include <vector>
#include <string>
#include "constants.h"
#include "IPLayer.h"


class AppLayer {

	private:
		IPLayer* ipLayer;

		void start();
		void sendData(std::string instruction[], int count);
		void getIPData();
		void getUserCommands();
		void printInterfaces();
		void printRoutes();
		void activateInterface(std::string itf);
		void deactivateInterface(std::string itf);
		void setMTU(std::string itf, std::string mtu);
		void toggleDebug();


	public:
		AppLayer(IPLayer* ipLayer);
		void run();
		static void *runListener(void* apl);
		void processCommand(const std::string& command);
		std::string* getAvailDes();
		int getDesCount();
		void increDesCount();
		void addDes(const std::string& des);
};

#endif
