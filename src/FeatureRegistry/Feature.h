#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

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

    void configureTask(uint16_t stackSize, BaseType_t core, UBaseType_t priority = 1)
    {
        _runAsTask = true;
        _taskStackSize = stackSize;
        _pinnedCore = core;
        _taskPriority = priority;
    }

    bool StartTask()
    {
        if (!_runAsTask || _taskHandle != nullptr)
            return false;

        _taskShouldStop = false;
        BaseType_t result = xTaskCreatePinnedToCore(
            taskEntry,
            _featureName.c_str(),
            _taskStackSize,
            this,
            _taskPriority,
            &_taskHandle,
            _pinnedCore);
        return result == pdPASS;
    }

    void StopTask()
    {
        if (!_taskHandle)
            return;
        _taskShouldStop = true;
        for (int i = 0; i < 50 && _taskHandle; i++)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        if (_taskHandle)
        {
            vTaskDelete(_taskHandle);
            _taskHandle = nullptr;
        }
    }

    bool IsTaskBased() const { return _runAsTask; }
    bool IsTaskRunning() const { return _taskHandle != nullptr && !_taskShouldStop; }
    TaskHandle_t GetTaskHandle() const { return _taskHandle; }

    UBaseType_t GetTaskStackHighWaterMark() const
    {
        if (_taskHandle)
            return uxTaskGetStackHighWaterMark(_taskHandle);
        return 0;
    }

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
        if (_taskHandle)
        {
            StopTask();
        }
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

    bool _runAsTask = false;
    volatile bool _taskShouldStop = false;
    TaskHandle_t _taskHandle = nullptr;
    uint16_t _taskStackSize = 4096;
    BaseType_t _pinnedCore = 0;
    UBaseType_t _taskPriority = 1;

    static void taskEntry(void *param)
    {
        Feature *self = static_cast<Feature *>(param);
        while (!self->_taskShouldStop)
        {
            if (self->_featureState == FeatureState::RUNNING)
            {
                self->_onLoop();
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        self->_taskHandle = nullptr;
        vTaskDelete(nullptr);
    }
};
