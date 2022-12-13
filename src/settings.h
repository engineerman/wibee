

#ifndef __SETTTINGS_H
#define __SETTTINGS_H

#include "WString.h"
#include "ArduinoJson.h"
#include "SD_MMC.h"

#define MAX_NUM_WIFI_CREDENTIALS 5
#define DEFAULT_WIBEE_NAME "wibee"

#define DEFAULT_WIFI_SSID "Vodafonenet_Wifi_0257"
#define DEFAULT_WIFI_PASS "LLXN3VRMYHMK"

#define CONFIGURATION_DATA_FILE "/config.json"
#define LOG_FILE_PATH "/data.log"

#define LOG_MEM_SIZE 16384
#define SD_CARD_WRITE_CHUNK_SIZE (4096)

#define DEBUG(x) Serial.println(x);

struct SpiRamAllocator
{
    void *allocate(size_t size)
    {
        return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    }

    void deallocate(void *pointer)
    {
        heap_caps_free(pointer);
    }

    void *reallocate(void *ptr, size_t new_size)
    {
        return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
    }
};

using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;

class systemConfiguration
{
private:
public:
    String mWiBeeName;
    String mPwd;

    String mWifiCredentials[MAX_NUM_WIFI_CREDENTIALS][2];

    systemConfiguration()
    {
        mWiBeeName = DEFAULT_WIBEE_NAME;
    }

    void checkDefaultWifiCredentials()
    {
        for (size_t m = 0; m < MAX_NUM_WIFI_CREDENTIALS; m++)
        {
            if (mWifiCredentials[m][0] == DEFAULT_WIFI_SSID && mWifiCredentials[m][1] == DEFAULT_WIFI_PASS)
            {
                return;
            }
        }

        if (!addWifiCredentials(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS))
        {
            addWifiCredentials(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS, 0);
        }
    }

    bool addWifiCredentials(String ssid, String pass, int index = -1)
    {
        if (ssid.isEmpty() || pass.isEmpty())
        {
            return false;
        }

        for (size_t m = 0; m < MAX_NUM_WIFI_CREDENTIALS; m++)
        {
            if (mWifiCredentials[m][0] == ssid && mWifiCredentials[m][1] == pass)
            {
                return true;
            }
        }

        if (index < 0)
        {
            for (size_t m = 0; m < MAX_NUM_WIFI_CREDENTIALS; m++)
            {
                if (mWifiCredentials[m][0].isEmpty())
                {
                    mWifiCredentials[m][0] = ssid;
                    mWifiCredentials[m][1] = pass;

                    // DEBUG(ssid + " added");

                    return true;
                }
            }
        }
        else if (index < MAX_NUM_WIFI_CREDENTIALS)
        {
            mWifiCredentials[index][0] = ssid;
            mWifiCredentials[index][1] = pass;

            return true;
        }

        DEBUG("Cannot Add WiFi");

        return false;
    }

    void clearWifiCredentials(int ind = -1)
    {
        if (ind < 0)
        {
            for (size_t m = 0; m < MAX_NUM_WIFI_CREDENTIALS; m++)
            {
                mWifiCredentials[m][0] = "";
                mWifiCredentials[m][1] = "";
            }
        }
        else if (ind < MAX_NUM_WIFI_CREDENTIALS)
        {
            mWifiCredentials[ind][0] = "";
            mWifiCredentials[ind][1] = "";
        }
    }

    ~systemConfiguration() {}

    String toJson(bool hidePassword = false)
    {
        String str = "{\n\
\"Name\": \"" +
                     mWiBeeName + "\",\n\
\"Pwd\": \"" + "*******\",\n\
\"WiFi\":\n\t[\n";

        bool first = true;

        for (size_t m = 0; m < MAX_NUM_WIFI_CREDENTIALS; m++)
        {
            if (!mWifiCredentials[m][0].isEmpty() && !mWifiCredentials[m][1].isEmpty())
            {
                if (!first)
                {
                    str += ",\n";
                }

                first = false;

                str += "\t\t[ \"" + mWifiCredentials[m][0] + "\",\"";
                str += hidePassword ? "*******" : mWifiCredentials[m][1];
                str += "\"]";
            }
        }

        str += "\n\t]\n}";

        return str;
    }

    bool fromJson(String json)
    {
        // Convert the Strign into JSON.
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, json);
        JsonObject obj = doc.as<JsonObject>();

        if (obj.containsKey("Name"))
        {
            mWiBeeName = obj["Name"].as<String>();
        }

        if (obj.containsKey("Pwd"))
        {
            mPwd = obj["Pwd"].as<String>();
        }

        if (obj.containsKey("WiFi"))
        {
            //  mTankLevelAlarmLiters = obj["TankLevelAlarmLiters"].as<int>();
            JsonArray arrayWifis = obj["WiFi"].as<JsonArray>();

            int numWifiCredentials = arrayWifis.size();

            if (numWifiCredentials > 0)
            {
                clearWifiCredentials();

                for (size_t i = 0; i < numWifiCredentials; i++)
                {
                    JsonArray arrCredentials = arrayWifis[i].as<JsonArray>();

                    if (arrCredentials.size() == 2)
                    {
                        String ssid = arrCredentials[0].as<String>();
                        String pass = arrCredentials[1].as<String>();

                        if (!ssid.isEmpty() && !pass.isEmpty())
                        {
                            addWifiCredentials(ssid, pass);
                        }
                    }
                }
            }
        }

        return true;
    }

    bool readFromFile(String filePath)
    {
        String json;

        fs::FS &fs = SD_MMC;
        // The calibration data is kept in a file in the SD.
        if (SD_MMC.exists(filePath))
        {
            File file = fs.open(filePath, FILE_READ);

            if (file)
            {
                while (file.available())
                {
                    char c = file.read();

                    json += c;
                }

                if (json != "")
                {
                    if (fromJson(json))
                    {
                        DEBUG("Config JSON loaded successfully");

                        return true;
                    }
                    else
                    {
                        DEBUG("Invalid Config JSON file.");

                        return false;
                    }
                }
                else
                {
                    DEBUG("Empty Config JSON file.");

                    return false;
                }
            }
            else
            {
                return false;
            }

            file.close();
        }
        else // If file does not exist, create a new one.
        {
            DEBUG("No Config data in the SD card...");

            File file = fs.open(filePath, FILE_WRITE);

            if (!file)
            {
                DEBUG("Failed to create new Config file");

                return false;
            }
            else
            {
                file.close();
            }

            DEBUG("Created a new empty file for Config.");

            return true;
        }
    }

    bool saveToFile(String filePath = CONFIGURATION_DATA_FILE)
    {
        String json = toJson();

        fs::FS &fs = SD_MMC;
        File file = fs.open(filePath, FILE_WRITE);

        if (!file)
        {
            DEBUG("Failed to open config file for writing");
            return false;
        }

        if (file.print(json))
        {
            DEBUG("config File written");
        }
        else
        {
            DEBUG("config Write failed");
        }

        file.close();

        return true;
    }

    void purge()
    {
        SD_MMC.remove(CONFIGURATION_DATA_FILE);
    }

    String toString(bool hidePassword = true)
    {
        String str = toJson(hidePassword);

        return str;
    }
};

#endif