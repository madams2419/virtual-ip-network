#ifndef APPLAYER_H
#define APPLAYER_H

#include <vector>
#include <string>
#include "constants.h"


class AppLayer {

	private:
		void start();
		void sendData(std::string instruction[]);
		std::string* availDes;
		int desCount;

	public:
		AppLayer();
		void runningApp(const std::string& command);
		std::string* getAvailDes();
		int getDesCount();
		void increDesCount();
		void addDes(const std::string& des);
};

#endif
