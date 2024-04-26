#pragma once
// SD-Card ////////////////////
#include "nvs_flash.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "driver/sdmmc_defs.h"
///////////////////////////////
#include <userGPIO.h>
#include <dirent.h>
#include <unistd.h>

#define MOUNT_POINT_SDCARD "/sdcard"
#define SSID "Hin"
#define PASSWORD "12341234"
#define MAX_CHAR_SIZE 64
const char *TAG = "T_SDCARD";
sdmmc_card_t *card = NULL;

// Write file example
static esp_err_t s_example_write_file(const char *path, char *data)
{
    ESP_LOGI(TAG, "Opening file %s", path);
    FILE *f = fopen(path, "w"); // Open the file. If not, create new file
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    return ESP_OK;
}

// Write file example
static esp_err_t write_file(const char *path, char *data)
{
    ESP_LOGI(TAG, "Opening file %s", path);
    FILE *f = fopen(path, "w"); // Open the file. If not, create new file
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    return ESP_OK;
}

static esp_err_t s_example_read_file(const char *path)
{
    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[MAX_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos)
    {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    return ESP_OK;
}

// Init SD card
bool initNVS_SDCard()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    printf("Using SDMMC peripheral \n");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;

    // GPIOs 15, 2, 4, 12, 13 should have external 10k pull-ups.
    // Internal pull-ups are not sufficient. However, enabling internal pull-ups
    // does make a difference some boards, so we do that here.
    gpio_set_pull_mode(GPIO_NUM_15, GPIO_PULLUP_ONLY); // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode(GPIO_NUM_2, GPIO_PULLUP_ONLY);  // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode(GPIO_NUM_4, GPIO_PULLUP_ONLY);  // D1, needed in 4-line mode only
    gpio_set_pull_mode(GPIO_NUM_12, GPIO_PULLUP_ONLY); // D2, needed in 4-line mode only
    gpio_set_pull_mode(GPIO_NUM_13, GPIO_PULLUP_ONLY); // D3, needed in 4- and 1-line modes

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
    // Please check its source code and implement error recovery when developing
    // production applications.

    ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            printf("Failed to mount filesystem. "
                   "If you want the card to be formatted, set format_if_mount_failed = true.");
        }
        else
        {
            printf("Failed to initialize the card (%s). "
                   "Make sure SD card lines have pull-up resistors in place.",
                   esp_err_to_name(ret));
        }
        return false;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Init the GPIO
    // Flash ausschalten
    gpio_reset_pin(FLASH_GPIO);
    gpio_set_direction(FLASH_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(FLASH_GPIO, 0);

    return true;
}

void createTreeFolder()
{
    const char *filePath = MOUNT_POINT_SDCARD "/test.txt";
    esp_err_t ret;

    // Check if destination file exists before creating
    struct stat st;
    if (stat(filePath, &st) != 0)
    {
        // Create a file
        char data[MAX_CHAR_SIZE];
        snprintf(data, MAX_CHAR_SIZE, "%s %s!\n", "Hello", card->cid.name);
        ret = s_example_write_file(filePath, data);
    }

    // Open file for reading
    while (1)
    {
        ret = s_example_read_file(filePath);
        if (ret != ESP_OK)
        {
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    // // All done, unmount partition and disable SDMMC peripheral
    // esp_vfs_fat_sdcard_unmount("/sdcard", card);
}

static void delete_file(const char *path)
{
    if (unlink(path) == 0)
    {
        ESP_LOGI(TAG, "Deleted file: %s", path);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to delete file: %s", path);
    }
}

// Delete all file
// e.g. delete_files_in_directory("/sdcard");
static void delete_files_in_directory(const char *dir_path)
{
    DIR *dir = opendir(dir_path);
    if (!dir)
    {
        ESP_LOGE(TAG, "Failed to open directory: %s", dir_path);
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // Bỏ qua thư mục gốc và thư mục cha
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            char file_path[255];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);
            delete_file(file_path);
        }
    }
    closedir(dir);
}

// Loop blink task if cannot init SD card
void taskNoSDBlink(void *pvParameter)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    TickType_t xDelay;
    xDelay = 100 / portTICK_PERIOD_MS;
    printf("SD-Card could not be inialized - STOP THE PROGRAMM HERE\n");

    while (1)
    {
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(xDelay);
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(xDelay);
    }
    vTaskDelete(NULL); // Delete this task if it exits from the loop above
}

void initTheContentSDCard()
{
    // 1. wlan.ini
    // ssid = "SSID"
    // password = "PASSWORD"
    // hostname = "watermeter"
    const char *filePath = MOUNT_POINT_SDCARD "/wlan.ini";
    char data[100]; // Chuỗi để chứa dữ liệu SSID, PASSWORD và hostname

    // Sử dụng sprintf() để tạo chuỗi với giá trị của SSID, PASSWORD và hostname
    sprintf(data, "ssid = \"%s\"\npassword = \"%s\"\nhostname = \"aiot-digi\"", SSID, PASSWORD);

    esp_err_t ret;

    // Check if destination file exists before creating
    struct stat st;
    if (stat(filePath, &st) != 0)
    {
        // Create a new file and override data
        ret = s_example_write_file(filePath, data);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to create wlan.ini file");
        }
    }
}