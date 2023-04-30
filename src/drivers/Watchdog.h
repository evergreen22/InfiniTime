#pragma once

#include <cstdint>
#include <nrf52_bitfields.h>

/*
 *  Low level driver for the watchdog based on the nRF52832 Product Specification V1.1
 *
 *  This driver initializes the timeout and sleep and halt behaviours of the watchdog
 *  in the method Watchdog::Setup().
 *
 *  The watchdog can then be started using the method Watchdog::Start(). At this point,
 *  the watchdog runs and will reset the MCU if it's not reloaded before the timeout elapses.
 *
 *  The watchdog can be reloaded using Watchdog::Kick()
 *
 *  The watchdog also provides the cause of the last reset (reset pin, watchdog, soft reset,
 *  hard reset,... see Watchdog::GetResetReasonString).
 */

namespace Pinetime {
  namespace Drivers {
    class Watchdog {
    public:
      enum class ResetReason { ResetPin, Watchdog, SoftReset, CpuLockup, SystemOff, LpComp, DebugInterface, NFC, HardReset };

      enum class SleepBehaviour : uint8_t {
        Pause = 0 << WDT_CONFIG_SLEEP_Pos,
        Run = 1 << WDT_CONFIG_SLEEP_Pos
      };

      enum class HaltBehaviour : uint8_t {
        Pause = 0 << WDT_CONFIG_HALT_Pos,
        Run = 1 << WDT_CONFIG_HALT_Pos
      };

      void Setup(uint8_t timeoutSeconds, SleepBehaviour sleepBehaviour, HaltBehaviour haltBehaviour);
      void Start();
      void Kick();
      const char* GetResetReasonString() const;

    private:
      ResetReason resetReason;
    };
  }
}
