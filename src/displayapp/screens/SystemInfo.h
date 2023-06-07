#pragma once

#include <memory>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"

namespace Pinetime {
  namespace Controllers {
    class DateTime;
    class Battery;
    class BrightnessController;
    class Ble;
  }

  namespace Drivers {
    class Watchdog;
  }

  namespace Applications {
    class DisplayApp;

    namespace Screens {
      class SystemInfo : public Screen {
      public:
        explicit SystemInfo(DisplayApp* app,
                            Controllers::DateTime& dateTimeController,
                            const Controllers::Battery& batteryController,
                            Controllers::BrightnessController& brightnessController,
                            const Controllers::Ble& bleController,
                            const Drivers::Watchdog& watchdog,
                            Controllers::MotionController& motionController,
                            const Drivers::Cst816S& touchPanel);
        ~SystemInfo() override;
        bool OnTouchEvent(TouchEvents event) override;

      private:
        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        Controllers::BrightnessController& brightnessController;
        const Controllers::Ble& bleController;
        const Drivers::Watchdog& watchdog;
        Controllers::MotionController& motionController;
        const Drivers::Cst816S& touchPanel;

        ScreenList<5> screens;

        static bool sortById(const TaskStatus_t& lhs, const TaskStatus_t& rhs);

        std::unique_ptr<Screen> CreateScreen1();
        std::unique_ptr<Screen> CreateScreen2();
        std::unique_ptr<Screen> CreateScreen3();
        std::unique_ptr<Screen> CreateScreen4();
        std::unique_ptr<Screen> CreateScreen5();
      };
    }
  }
}
