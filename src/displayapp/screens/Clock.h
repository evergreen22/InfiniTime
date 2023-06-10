#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "components/heartrate/HeartRateController.h"
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {
      class Clock : public Screen {
      public:
        Clock(DisplayApp* app,
              Controllers::DateTime& dateTimeController,
              const Controllers::Battery& batteryController,
              const Controllers::Ble& bleController,
              Controllers::NotificationManager& notificatioManager,
              Controllers::Settings& settingsController,
              Controllers::HeartRateController& heartRateController,
              Controllers::MotionController& motionController);
        ~Clock() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::NotificationManager& notificatioManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;

        std::unique_ptr<Screen> screen;
        std::unique_ptr<Screen> WatchFaceDigitalScreen();
        std::unique_ptr<Screen> WatchFaceAnalogScreen();
      };
    }
  }
}
