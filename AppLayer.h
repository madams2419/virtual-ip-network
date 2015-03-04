#ifndef APPLAYER_H
#define APPLAYER_H

#include <vector>
#include <string>
#include "constants.h"
#include "IPLayer.h"


class AppLayer {

	private:
		void start();
		void sendData(std::string instruction[], int count);
		std::string* availDes;
		int desCount;
		IPLayer* ipLayer;

	public:
		AppLayer(IPLayer* ipLayer);
		void runningApp(const std::string& command);
		std::string* getAvailDes();
		int getDesCount();
		void increDesCount();
		void addDes(const std::string& des);
};

#endif
