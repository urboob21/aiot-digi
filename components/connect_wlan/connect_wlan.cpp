#include "connect_wlan.h"



#include <string.h>
#include "esp_log.h"

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include <arpa/inet.h>

#include "Helper.h"

static const char *MAIN_TAG = "connect_wlan";

std::string ssid = "";
std::string passphrase = "";
std::string hostname = "";
std::string ipaddress = "";
std::string gw = "";
std::string netmask = "";
std::string dns = "";
std::string std_hostname = "watermeter";

static EventGroupHandle_t wifi_event_group;


#define BLINK_GPIO GPIO_NUM_33


std::vector<string> ZerlegeZeile(std::string input, std::string _delimiter = "")
{
	std::vector<string> Output;
	std::string delimiter = " =,";
    if (_delimiter.length() > 0){
        delimiter = _delimiter;
    }

	input = trim(input, delimiter);
	size_t pos = findDelimiterPos(input, delimiter);
	std::string token;
	while (pos != std::string::npos) {
		token = input.substr(0, pos);
		token = trim(token, delimiter);
		Output.push_back(token);
		input.erase(0, pos + 1);
		input = trim(input, delimiter);
		pos = findDelimiterPos(input, delimiter);
	}
	Output.push_back(input);

	return Output;
}

void loadWlanFromFile(std::string fn)
{
    string line = "";
    std::vector<string> zerlegt;
    hostname = std_hostname;

    FILE* pFile;
    fn = FormatFileName(fn);

    pFile = OpenFileAndWait(fn.c_str(), "r");
    printf("file loaded\n");

    if (pFile == NULL)
        return;

    char zw[1024];
    fgets(zw, 1024, pFile);
    line = std::string(zw);

    while ((line.size() > 0) || !(feof(pFile)))
    {
        printf("%s", line.c_str());
        zerlegt = ZerlegeZeile(line, "=");
        zerlegt[0] = trim(zerlegt[0], " ");
        for (int i = 2; i < zerlegt.size(); ++i)
            zerlegt[i] = zerlegt[i-1] + zerlegt[i];

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "HOSTNAME")){
            hostname = trim(zerlegt[1]);
            if ((hostname[0] == '"') && (hostname[hostname.length()-1] == '"')){
                hostname = hostname.substr(1, hostname.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "SSID")){
            ssid = trim(zerlegt[1]);
            if ((ssid[0] == '"') && (ssid[ssid.length()-1] == '"')){
                ssid = ssid.substr(1, ssid.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "PASSWORD")){
            passphrase = zerlegt[1];
            if ((passphrase[0] == '"') && (passphrase[passphrase.length()-1] == '"')){
                passphrase = passphrase.substr(1, passphrase.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "IP")){
            ipaddress = zerlegt[1];
            if ((ipaddress[0] == '"') && (ipaddress[ipaddress.length()-1] == '"')){
                ipaddress = ipaddress.substr(1, ipaddress.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "GATEWAY")){
            gw = zerlegt[1];
            if ((gw[0] == '"') && (gw[gw.length()-1] == '"')){
                gw = gw.substr(1, gw.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "NETMASK")){
            netmask = zerlegt[1];
            if ((netmask[0] == '"') && (netmask[netmask.length()-1] == '"')){
                netmask = netmask.substr(1, netmask.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "DNS")){
            dns = zerlegt[1];
            if ((dns[0] == '"') && (dns[dns.length()-1] == '"')){
                dns = dns.substr(1, dns.length()-2);
            }
        }


        if (fgets(zw, 1024, pFile) == NULL)
        {
            line = "";
        }
        else
        {
            line = std::string(zw);
        }
    }

    fclose(pFile);

    // Check if Hostname was empty in .ini if yes set to std_hostname
    if(hostname.length() <= 0){
        hostname = std_hostname;
    }

    while(1){
    printf("\nWLan: %s, %s\n", ssid.c_str(), passphrase.c_str());
    printf("Hostename: %s\n", hostname.c_str());
    printf("Fixed IP: %s, Gateway %s, Netmask %s, DNS %s\n", ipaddress.c_str(), gw.c_str(), netmask.c_str(), dns.c_str());
    vTaskDelay(pdTICKS_TO_MS(1000));
    }


}



std::string getHostname(){
    return hostname;
}

std::string getIPAddress(){
    return ipaddress;
}

std::string getSSID(){
    return ssid;
}

std::string getNetMask(){
    return netmask;
}

std::string getGW(){
    return gw;
}

