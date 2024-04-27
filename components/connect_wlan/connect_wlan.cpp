#include "connect_wlan.h"

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include <arpa/inet.h>
#include "Helper.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
static const char *MAIN_TAG = "connect_wlan";

std::string ssid = "";
std::string passphrase = "";
std::string hostname = "";
std::string ipaddress = "";
std::string gw = "";
std::string netmask = "";
std::string dns = "";
std::string std_hostname = "watermeter";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

#define BLINK_GPIO GPIO_NUM_33
/* The event group allows multiple bits for each event, but we only care about two events:*/
#define CONNECTED_BIT BIT0
std::vector<string> ZerlegeZeile(std::string input, std::string _delimiter = "")
{
    std::vector<string> Output;
    std::string delimiter = " =,";
    if (_delimiter.length() > 0)
    {
        delimiter = _delimiter;
    }

    input = trim(input, delimiter);
    size_t pos = findDelimiterPos(input, delimiter);
    std::string token;
    while (pos != std::string::npos)
    {
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

void blinkstatus(int dauer, int _anzahl)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    for (int i = 0; i < _anzahl; ++i)
    {
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(dauer / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(dauer / portTICK_PERIOD_MS);          
    }
}

void wifi_connect(){
    wifi_config_t cfg = { };
    strcpy((char*)cfg.sta.ssid, (const char*)ssid.c_str());
    strcpy((char*)cfg.sta.password, (const char*)passphrase.c_str());
    
    ESP_ERROR_CHECK( esp_wifi_disconnect() );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg) );
    ESP_ERROR_CHECK( esp_wifi_connect() );
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        blinkstatus(200, 1);
        wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        blinkstatus(200, 5);
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        blinkstatus(1000, 3);
    }
}

void strinttoip4(std::string ip, int &a, int &b, int &c, int &d) {
    std::stringstream s(ip);
    char ch; //to temporarily store the '.'
    s >> a >> ch >> b >> ch >> c >> ch >> d;
}

void loadWlanFromFile(std::string fn)
{
    string line = "";
    std::vector<string> zerlegt;
    hostname = std_hostname;

    FILE *pFile;
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
            zerlegt[i] = zerlegt[i - 1] + zerlegt[i];

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "HOSTNAME"))
        {
            hostname = trim(zerlegt[1]);
            if ((hostname[0] == '"') && (hostname[hostname.length() - 1] == '"'))
            {
                hostname = hostname.substr(1, hostname.length() - 2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "SSID"))
        {
            ssid = trim(zerlegt[1]);
            if ((ssid[0] == '"') && (ssid[ssid.length() - 1] == '"'))
            {
                ssid = ssid.substr(1, ssid.length() - 2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "PASSWORD"))
        {
            passphrase = zerlegt[1];
            if ((passphrase[0] == '"') && (passphrase[passphrase.length() - 1] == '"'))
            {
                passphrase = passphrase.substr(1, passphrase.length() - 2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "IP"))
        {
            ipaddress = zerlegt[1];
            if ((ipaddress[0] == '"') && (ipaddress[ipaddress.length() - 1] == '"'))
            {
                ipaddress = ipaddress.substr(1, ipaddress.length() - 2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "GATEWAY"))
        {
            gw = zerlegt[1];
            if ((gw[0] == '"') && (gw[gw.length() - 1] == '"'))
            {
                gw = gw.substr(1, gw.length() - 2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "NETMASK"))
        {
            netmask = zerlegt[1];
            if ((netmask[0] == '"') && (netmask[netmask.length() - 1] == '"'))
            {
                netmask = netmask.substr(1, netmask.length() - 2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "DNS"))
        {
            dns = zerlegt[1];
            if ((dns[0] == '"') && (dns[dns.length() - 1] == '"'))
            {
                dns = dns.substr(1, dns.length() - 2);
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
    if (hostname.length() <= 0)
    {
        hostname = std_hostname;
    }

    while (1)
    {
        printf("\nWLan: %s, %s\n", ssid.c_str(), passphrase.c_str());
        printf("Hostename: %s\n", hostname.c_str());
        printf("Fixed IP: %s, Gateway %s, Netmask %s, DNS %s\n", ipaddress.c_str(), gw.c_str(), netmask.c_str(), dns.c_str());
        vTaskDelay(pdTICKS_TO_MS(1000));
    }
}

void initialise_wifi_fixed_ip()
{
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *my_sta = esp_netif_create_default_wifi_sta();

    esp_netif_dhcpc_stop(my_sta);

    esp_netif_ip_info_t ip_info;

    int a, b, c, d;

    strinttoip4(ipaddress, a, b, c, d);
    IP4_ADDR(&ip_info.ip, a, b, c, d);

    strinttoip4(gw, a, b, c, d);
    IP4_ADDR(&ip_info.gw, a, b, c, d);

    strinttoip4(netmask, a, b, c, d);
    IP4_ADDR(&ip_info.netmask, a, b, c, d);

    esp_netif_set_ip_info(my_sta, &ip_info);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    if (dns.length() > 0)
    {
        esp_netif_dns_info_t dns_info;
        ip4_addr_t ip;
        ip.addr = esp_ip4addr_aton(dns.c_str());
        ip_addr_set_ip4_u32(&dns_info.ip, ip.addr);
        ESP_ERROR_CHECK(esp_netif_set_dns_info(my_sta, ESP_NETIF_DNS_MAIN, &dns_info));
    }

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {};
    strcpy((char *)wifi_config.sta.ssid, (const char *)ssid.c_str());
    strcpy((char *)wifi_config.sta.password, (const char *)passphrase.c_str());

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(MAIN_TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, true, true, portMAX_DELAY);

    if (bits & CONNECTED_BIT)
    {
        ESP_LOGI(MAIN_TAG, "connected to ap SSID:%s password:%s",
                 ssid.c_str(), passphrase.c_str());
    }
    else
    {
        ESP_LOGI(MAIN_TAG, "Failed to connect to SSID:%s, password:%s",
                 ssid.c_str(), passphrase.c_str());
    }

    tcpip_adapter_ip_info_t ip_info2;
    ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info2));
    ipaddress = std::string(ip4addr_ntoa(&ip_info2.ip));
    netmask = std::string(ip4addr_ntoa(&ip_info2.netmask));
    gw = std::string(ip4addr_ntoa(&ip_info2.gw));
}

void connectToWLAN()
{
    printf("Connect to WLAN with fixed IP\n");
    initialise_wifi_fixed_ip();
}

std::string getHostname()
{
    return hostname;
}

std::string getIPAddress()
{
    return ipaddress;
}

std::string getSSID()
{
    return ssid;
}

std::string getNetMask()
{
    return netmask;
}

std::string getGW()
{
    return gw;
}
