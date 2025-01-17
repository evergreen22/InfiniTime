#pragma once
#include <FreeRTOS.h>
#include <bits/unique_ptr.h>
#include <queue.h>
#include <task.h>
#include <date/date.h>
#include "drivers/St7789.h"
#include "drivers/SpiMaster.h"
#include "components/gfx/Gfx.h"
#include "drivers/Cst816s.h"
#include "drivers/Watchdog.h"
#include "components/motor/MotorController.h"
#include "BootErrors.h"
#include "displayapp/TouchEvents.h"
#include "displayapp/Apps.h"
#include "displayapp/Messages.h"
#include "displayapp/DummyLittleVgl.h"

namespace Pinetime {
  namespace Drivers {
    class St7789;
    class Cst816S;
    class Watchdog;
  }
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class DateTime;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
    class TouchHandler;
    class MotorController;
    class TimerController;
    class AlarmController;
  }

  namespace System {
    class SystemTask;
  };

  namespace Applications {
    class DisplayApp {
    public:
      DisplayApp(Drivers::St7789& lcd,
                 Components::LittleVgl& lvgl,
                 const Drivers::Cst816S&,
                 const Controllers::Battery& batteryController,
                 constControllers::Ble& bleController,
                 Controllers::DateTime& dateTimeController,
                 const Drivers::Watchdog& watchdog,
                 Controllers::NotificationManager& notificationManager,
                 Controllers::HeartRateController& heartRateController,
                 Controllers::Settings& settingsController,
                 Controllers::MotorController& motorController,
                 Controllers::MotionController& motionController,
                 Controllers::TimerController& timerController,
                 Controllers::AlarmController& alarmController,
                 Controllers::TouchHandler& touchHandler);
      void Start();
      void Start(Pinetime::System::BootErrors){ Start(); };
      void PushMessage(Pinetime::Applications::Display::Messages msg);
      void Register(Pinetime::System::SystemTask* systemTask);

    private:
      TaskHandle_t taskHandle;
      static void Process(void* instance);
      void DisplayLogo(uint16_t color);
      void DisplayOtaProgress(uint8_t percent, uint16_t color);
      void InitHw();
      void Refresh();
      Drivers::St7789& lcd;
      const Controllers::Ble& bleController;

      static constexpr uint8_t queueSize = 10;
      static constexpr uint8_t itemSize = 1;
      QueueHandle_t msgQueue;
      static constexpr uint8_t displayWidth = 240;
      static constexpr uint8_t displayHeight = 240;
      static constexpr uint8_t bytesPerPixel = 2;

      static constexpr uint16_t colorWhite = 0xFFFF;
      static constexpr uint16_t colorGreen = 0x07E0;
      static constexpr uint16_t colorGreenSwapped = 0xE007;
      static constexpr uint16_t colorBlue = 0x0000ff;
      static constexpr uint16_t colorRed = 0xff00;
      static constexpr uint16_t colorRedSwapped = 0x00ff;
      static constexpr uint16_t colorBlack = 0x0000;
      uint8_t displayBuffer[displayWidth * bytesPerPixel];
    };
  }
}
