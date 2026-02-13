#pragma once

enum class FeatureState
{
    PENDING = 0,
    SETUP = 1,
    RUNNING = 2,
    ERROR = 3
};

typedef enum FeatureState (*FeatureSetupFunction)();

typedef void (*FeatureLoopFunction)();

class Feature
{
public:
    Feature(String name = "featureName", FeatureSetupFunction setupCallback = []()
                                         { return FeatureState::PENDING; },
            FeatureLoopFunction loopCallback = []() {}) : _featureName(name), _onSetup(setupCallback), _onLoop(loopCallback) {};

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

    String GetFeatureName()
    {
        return this->_featureName;
    }

    FeatureState GetFeatureState()
    {
        return this->_featureState;
    }

protected:
    String _featureName;
    FeatureSetupFunction _onSetup;
    FeatureLoopFunction _onLoop;
    FeatureState _featureState;
};
