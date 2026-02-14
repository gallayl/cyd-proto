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

#if ENABLE_PIR_SENSOR
#include "./Features/Pir.h"
#endif

#if ENABLE_I2C
#include "./Features/i2c.h"

#endif

#if ENABLE_OTA
#include "./Features/OTA.h"
#endif

#if ENABLE_UI
#include "./Features/UI/UiFeature.h"
#endif

#define FEATURES_SIZE 16

extern Feature *SystemFeatures;

class FeatureRegistry
{
private:
        uint8_t _registeredFeaturesCount = 0;

        void updateFeatureJson(Feature *feature)
        {
                const String &name = feature->GetFeatureName();
                JsonObject entry = registeredFeatures[name];
                if (!entry.isNull())
                {
                        entry["state"].set((int)feature->GetFeatureState());
                }
        }

public:
        FeatureRegistry() {}

        void Init()
        {
                this->RegisterFeature(TimeFeature);
                this->RegisterFeature(LoggingFeature);
                this->RegisterFeature(SystemFeatures);

#if ENABLE_SERIAL_READ
                this->RegisterFeature(SerialReadFeature);
#endif

#if ENABLE_LITTLEFS
                this->RegisterFeature(LittleFsFeature);
#endif

#if ENABLE_PIR_SENSOR
                this->RegisterFeature(PirFeature);
#endif

#if ENABLE_I2C
                this->RegisterFeature(i2cFeature);
#endif

#if ENABLE_OTA
                this->RegisterFeature(OtaUpgrade);
#endif

#if ENABLE_UI
                this->RegisterFeature(UiFeature);
#endif
        }

        void RegisterFeature(Feature *newFeature)
        {
                if (this->_registeredFeaturesCount >= FEATURES_SIZE)
                {
                        LoggerInstance->Error(F("Feature registry full, cannot register"));
                        return;
                }
                this->RegisteredFeatures[this->_registeredFeaturesCount] = newFeature;
                this->_registeredFeaturesCount++;
                const String &featureName = newFeature->GetFeatureName();

                JsonObject featureEntry = registeredFeatures[featureName].to<JsonObject>();
                featureEntry["name"] = featureName;
                featureEntry["state"] = (int)newFeature->GetFeatureState();
        }

        void SetupFeatures()
        {
                for (uint8_t i = 0; i < this->_registeredFeaturesCount; i++)
                {
                        Feature *f = this->RegisteredFeatures[i];
                        const String &featureName = f->GetFeatureName();

                        if (!f->isAutoStart())
                        {
                                LoggerInstance->Info("Deferring feature: " + featureName);
                                continue;
                        }

                        LoggerInstance->Info("Setting up feature: " + featureName);
                        f->Setup();
                        updateFeatureJson(f);
                }

#if ENABLE_WEBSERVER
                ActionRegistryInstance->WireRestEndpoints();
#endif
        }

        void LoopFeatures()
        {
                for (uint8_t i = 0; i < this->_registeredFeaturesCount; i++)
                {
                        if (this->RegisteredFeatures[i]->GetFeatureState() == FeatureState::RUNNING)
                        {
                                this->RegisteredFeatures[i]->Loop();
                        }
                }
        }

        Feature *GetFeature(const String &name)
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

        FeatureState SetupFeature(const String &name)
        {
                Feature *f = GetFeature(name);
                if (!f)
                {
                        LoggerInstance->Error("Feature not found: " + name);
                        return FeatureState::ERROR;
                }

                FeatureState state = f->GetFeatureState();
                if (state == FeatureState::RUNNING)
                {
                        LoggerInstance->Info("Feature already running: " + name);
                        return state;
                }

                LoggerInstance->Info("Setting up feature: " + name);
                FeatureState newState = f->Setup();
                updateFeatureJson(f);
                return newState;
        }

        bool TeardownFeature(const String &name)
        {
                Feature *f = GetFeature(name);
                if (!f)
                {
                        LoggerInstance->Error("Feature not found: " + name);
                        return false;
                }

                if (f->GetFeatureState() != FeatureState::RUNNING)
                {
                        LoggerInstance->Info("Feature not running: " + name);
                        return false;
                }

                if (!f->hasTeardown())
                {
                        LoggerInstance->Error("Feature has no teardown: " + name);
                        return false;
                }

                LoggerInstance->Info("Tearing down feature: " + name);
                f->Teardown();
                updateFeatureJson(f);
                return true;
        }

        uint8_t GetFeatureCount() const
        {
                return this->_registeredFeaturesCount;
        }

        Feature *RegisteredFeatures[FEATURES_SIZE] = {};
};

extern FeatureRegistry *FeatureRegistryInstance;
