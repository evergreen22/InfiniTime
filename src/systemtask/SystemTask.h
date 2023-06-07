#pragma once

#include <memory>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include "heartratetask/HeartRateTask.h"
#include "components/settings/Settings.h"
#include "drivers/Bma421.h"
#include "drivers/PinMap.h"
#include "components/motion/MotionController.h"

#include "systemtask/SystemMonitor.h"
#include "components/battery/BatteryController.h"
#include "components/ble/NimbleController.h"
#include "components/ble/NotificationManager.h"
#include "components/motor/MotorController.h"
#include "components/timer/TimerController.h"
#include "components/alarm/AlarmController.h"
#include "components/fs/FS.h"
#include "touchhandler/TouchHandler.h"
#include "buttonhandler/ButtonHandler.h"
#include "buttonhandler/ButtonActions.h"

#ifdef PINETIME_IS_RECOVERY
  #include "displayapp/DisplayAppRecovery.h"
  #include "displayapp/DummyLittleVgl.h"
#else
  #include "components/settings/Settings.h"
  #include "displayapp/DisplayApp.h"
  #include "displayapp/LittleVgl.h"
#endif

#include "drivers/Watchdog.h"
#include "systemtask/Messages.h"

extern std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> NoInit_BackUpTime;
namespace Pinetime {
  namespace Drivers {
    class Cst816S;
    class SpiMaster;
    class SpiNorFlash;
    class St7789;
    class TwiMaster;
    class Hrs3300;
  }
  namespace Controllers {
    class TouchHandler;
    class ButtonHandler;
  }
  namespace System {
    class SystemTask {
    public:
      SystemTask(Drivers::SpiMaster& spi,
                 Drivers::St7789& lcd,
                 Drivers::SpiNorFlash& spiNorFlash,
                 Drivers::TwiMaster& twiMaster,
                 Drivers::Cst816S& touchPanel,
                 Components::LittleVgl& lvgl,
                 Controllers::Battery& batteryController,
                 Controllers::Ble& bleController,
                 Controllers::DateTime& dateTimeController,
                 Controllers::TimerController& timerController,
                 Controllers::AlarmController& alarmController,
                 Drivers::Watchdog& watchdog,
                 Controllers::NotificationManager& notificationManager,
                 Controllers::MotorController& motorController,
                 Drivers::Hrs3300& heartRateSensor,
                 Controllers::MotionController& motionController,
                 Drivers::Bma421& motionSensor,
                 Controllers::Settings& settingsController,
                 Controllers::HeartRateController& heartRateController,
                 Pinetime::Applications::DisplayApp& displayApp,
                 Pinetime::Applications::HeartRateTask& heartRateApp,
                 Controllers::FS& fs,
                 Controllers::TouchHandler& touchHandler,
                 Controllers::ButtonHandler& buttonHandler);

      void Start();
      void PushMessage(Messages msg);

      void OnTouchEvent();

      void OnIdle();
      void OnDim();

      Pinetime::Controllers::NimbleController& nimble() {
        return nimbleController;
      };

      bool IsSleeping() const {
        return isSleeping;
      }

    private:
      TaskHandle_t taskHandle;

      Drivers::SpiMaster& spi;
      Drivers::St7789& lcd;
      Drivers::SpiNorFlash& spiNorFlash;
      Drivers::TwiMaster& twiMaster;
      Drivers::Cst816S& touchPanel;
      Components::LittleVgl& lvgl;
      Controllers::Battery& batteryController;

      Controllers::Ble& bleController;
      Controllers::DateTime& dateTimeController;
      Controllers::TimerController& timerController;
      Controllers::AlarmController& alarmController;
      QueueHandle_t systemTasksMsgQueue;
      std::atomic<bool> isSleeping {false};
      std::atomic<bool> isGoingToSleep {false};
      std::atomic<bool> isWakingUp {false};
      std::atomic<bool> isDimmed {false};
      Drivers::Watchdog& watchdog;
      Controllers::NotificationManager& notificationManager;
      Controllers::MotorController& motorController;
      Drivers::Hrs3300& heartRateSensor;
      Drivers::Bma421& motionSensor;
      Controllers::Settings& settingsController;
      Controllers::HeartRateController& heartRateController;
      Controllers::MotionController& motionController;

      Pinetime::Applications::DisplayApp& displayApp;
      Pinetime::Applications::HeartRateTask& heartRateApp;
      Controllers::FS& fs;
      Controllers::TouchHandler& touchHandler;
      Controllers::ButtonHandler& buttonHandler;
      Controllers::NimbleController nimbleController;

      static void Process(void* instance);
      void Work();
      void ReloadIdleTimer();
      bool isBleDiscoveryTimerRunning = false;
      uint8_t bleDiscoveryTimer = 0;
      TimerHandle_t dimTimer;
      TimerHandle_t idleTimer;
      TimerHandle_t measureBatteryTimer;
      bool doNotGoToSleep = false;

      void HandleButtonAction(Controllers::ButtonActions action);
      bool fastWakeUpDone = false;

      void GoToRunning();
      void UpdateMotion();
      bool stepCounterMustBeReset = false;
      static constexpr TickType_t batteryMeasurementPeriod = pdMS_TO_TICKS(10 * 60 * 1000);

#if configUSE_TRACE_FACILITY == 1
      SystemMonitor<FreeRtosMonitor> monitor;
#else
      SystemMonitor<DummyMonitor> monitor;
#endif
    };
  }
}
