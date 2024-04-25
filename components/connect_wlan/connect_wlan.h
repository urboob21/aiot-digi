#ifndef CONNECT_WLAN_H
#define CONNECT_WLAN_H

#include <string>

void loadWlanFromFile(std::string fn);

std::string getHostname();
std::string getIPAddress();
std::string getSSID();
std::string getNetMask();
std::string getGW();

#endif