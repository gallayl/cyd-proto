#pragma once

#include <Arduino.h>

enum class FeatureState
{
    PENDING = 0,
    SETUP = 1,
    RUNNING = 2,
    ERROR = 3,
    STOPPED = 4
};

typedef enum FeatureState (*FeatureSetupFunction)();

typedef void (*FeatureLoopFunction)();

typedef void (*FeatureTeardownFunction)();

class Feature
{
public:
    Feature(
        const String &name = "featureName", FeatureSetupFunction setupCallback = []() { return FeatureState::PENDING; },
        FeatureLoopFunction loopCallback = []() {}, FeatureTeardownFunction teardownCallback = nullptr,
        bool autoStart = true)
        : _featureName(name), _onSetup(setupCallback), _onLoop(loopCallback), _onTeardown(teardownCallback),
          _autoStart(autoStart) {};

    FeatureState Setup()
    {
        this->_featureState = FeatureState::SETUP;
        FeatureState newState = this->_onSetup();
        this->_featureState = newState;
        return newState;
    }

    void Loop()
    {
        return this->_onLoop();
    }

    void Teardown()
    {
        if (this->_onTeardown)
        {
            this->_onTeardown();
        }
        this->_featureState = FeatureState::STOPPED;
    }

    const String &GetFeatureName() const
    {
        return this->_featureName;
    }

    FeatureState GetFeatureState() const
    {
        return this->_featureState;
    }

    bool isAutoStart() const
    {
        return this->_autoStart;
    }

    void setAutoStart(bool autoStart)
    {
        this->_autoStart = autoStart;
    }

    bool hasTeardown() const
    {
        return this->_onTeardown != nullptr;
    }

protected:
    String _featureName;
    FeatureSetupFunction _onSetup;
    FeatureLoopFunction _onLoop;
    FeatureTeardownFunction _onTeardown;
    FeatureState _featureState = FeatureState::PENDING;
    bool _autoStart;
};
