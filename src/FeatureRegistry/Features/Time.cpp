#include "Time.h"

// implementations

time_t getEpochTime()
{
    time_t now;
    time(&now);
    return now;
}

std::string getUtcTime()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = gmtime(&rawtime);

    strftime(buffer, 80, "%FT%TZ", timeinfo);
    return buffer;
}

// instantiate feature
Feature *timeFeature = new Feature(
    "Time",
    []()
    {
        esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, MY_NTP_SERVER);
        esp_sntp_init();
        /* set the TZ environment variable so localtime() works with our string */
        setenv("TZ", MY_TZ, 1);
        tzset();
        return FeatureState::RUNNING;
    },
    []() {});
