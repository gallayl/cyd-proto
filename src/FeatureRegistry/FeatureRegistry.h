#pragma once

#include "../config.h"
#include "./registeredFeatures.h"
#include "./Feature.h"
#include "../ActionRegistry/ActionRegistry.h"
#include "./Features/Time.h"
#include "./Features/Logging.h"
#include "./Features/SystemFeatures.h"

#if ENABLE_SERIAL_READ
#include "./Features/SerialRead.h"
#endif

#if ENABLE_LITTLEFS
#include "./Features/LittleFsManagement/LittleFsManagement.h"
#endif

#if ENABLE_I2C
#include "./Features/i2c.h"

#endif

#if ENABLE_SD_CARD
#include "./Features/SdCard/SdCardFeature.h"
#endif

#if ENABLE_OTA
#include "./Features/OTA.h"
#endif

#if ENABLE_UI
#include "./Features/UI/UiFeature.h"
#endif

#if ENABLE_BERRY
#include "./Features/Berry/BerryFeature.h"
#endif

#include <mutex>

#define FEATURES_SIZE 16

class FeatureRegistry
{
private:
    uint8_t _registeredFeaturesCount = 0;

    void updateFeatureJson(Feature *feature)
    {
        std::lock_guard<std::mutex> lock(registeredFeaturesMutex);
        const String &name = feature->GetFeatureName();
        JsonObject entry = registeredFeatures[name];
        if (!entry.isNull())
        {
            entry["state"].set((int)feature->GetFeatureState());
        }
    }

public:
    FeatureRegistry() = default;

    void init()
    {
        this->registerFeature(timeFeature);
        this->registerFeature(loggingFeature);
        this->registerFeature(systemFeatures);

#if ENABLE_SERIAL_READ
        this->registerFeature(serialReadFeature);
#endif

#if ENABLE_LITTLEFS
        this->registerFeature(LittleFsFeature);
#endif

#if ENABLE_SD_CARD
        this->registerFeature(SdCardFeature);
#endif

#if ENABLE_I2C
        this->registerFeature(i2cFeature);
#endif

#if ENABLE_OTA
        this->registerFeature(otaUpgrade);
#endif

#if ENABLE_UI
        this->registerFeature(uiFeature);
#endif

#if ENABLE_BERRY
        this->registerFeature(BerryFeature);
#endif
    }

    void registerFeature(Feature *newFeature)
    {
        std::lock_guard<std::mutex> lock(registeredFeaturesMutex);
        if (this->_registeredFeaturesCount >= FEATURES_SIZE)
        {
            loggerInstance->Error(F("Feature registry full, cannot register"));
            return;
        }
        this->RegisteredFeatures[this->_registeredFeaturesCount] = newFeature;
        this->_registeredFeaturesCount++;
        const String &featureName = newFeature->GetFeatureName();

        JsonObject featureEntry = registeredFeatures[featureName].to<JsonObject>();
        featureEntry["name"] = featureName;
        featureEntry["state"] = (int)newFeature->GetFeatureState();
    }

    void setupFeatures()
    {
        for (uint8_t i = 0; i < this->_registeredFeaturesCount; i++)
        {
            Feature *f = this->RegisteredFeatures[i];
            const String &featureName = f->GetFeatureName();

            if (!f->isAutoStart())
            {
                loggerInstance->Info("Deferring feature: " + featureName);
                continue;
            }

            loggerInstance->Info("Setting up feature: " + featureName);
            f->Setup();
            updateFeatureJson(f);
        }

#if ENABLE_WEBSERVER
        actionRegistryInstance->wireRestEndpoints();
#endif
    }

    void startFeatureTasks()
    {
        for (uint8_t i = 0; i < this->_registeredFeaturesCount; i++)
        {
            Feature *f = this->RegisteredFeatures[i];
            if (f->isTaskBased() && f->GetFeatureState() == FeatureState::RUNNING)
            {
                if (f->startTask())
                {
                    loggerInstance->Info("Started task for: " + f->GetFeatureName());
                }
                else
                {
                    loggerInstance->Error("Failed to start task for: " + f->GetFeatureName());
                }
            }
        }
    }

    void loopFeatures()
    {
        for (uint8_t i = 0; i < this->_registeredFeaturesCount; i++)
        {
            Feature *f = this->RegisteredFeatures[i];
            if (f->isTaskBased())
            {
                continue;
            }
            if (f->GetFeatureState() == FeatureState::RUNNING)
            {
                f->Loop();
            }
        }
    }

    Feature *getFeature(const String &name)
    {
        for (uint8_t i = 0; i < this->_registeredFeaturesCount; i++)
        {
            if (this->RegisteredFeatures[i]->GetFeatureName() == name)
            {
                return this->RegisteredFeatures[i];
            }
        }
        return nullptr;
    }

    FeatureState setupFeature(const String &name)
    {
        Feature *f = getFeature(name);
        if (f == nullptr)
        {
            loggerInstance->Error("Feature not found: " + name);
            return FeatureState::ERROR;
        }

        FeatureState state = f->GetFeatureState();
        if (state == FeatureState::RUNNING)
        {
            loggerInstance->Info("Feature already running: " + name);
            return state;
        }

        loggerInstance->Info("Setting up feature: " + name);
        FeatureState newState = f->Setup();
        updateFeatureJson(f);

        if (f->isTaskBased() && newState == FeatureState::RUNNING)
        {
            f->startTask();
        }

        return newState;
    }

    bool teardownFeature(const String &name)
    {
        Feature *f = getFeature(name);
        if (f == nullptr)
        {
            loggerInstance->Error("Feature not found: " + name);
            return false;
        }

        if (f->GetFeatureState() != FeatureState::RUNNING)
        {
            loggerInstance->Info("Feature not running: " + name);
            return false;
        }

        if (!f->hasTeardown())
        {
            loggerInstance->Error("Feature has no teardown: " + name);
            return false;
        }

        loggerInstance->Info("Tearing down feature: " + name);
        f->Teardown();
        updateFeatureJson(f);
        return true;
    }

    uint8_t getFeatureCount() const
    {
        return this->_registeredFeaturesCount;
    }

    Feature *RegisteredFeatures[FEATURES_SIZE] = {};
};

extern FeatureRegistry *featureRegistryInstance;
