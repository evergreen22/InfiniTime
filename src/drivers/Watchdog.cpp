#include "drivers/Watchdog.h"
#include <mdk/nrf.h>
using namespace Pinetime::Drivers;

namespace {
  constexpr uint32_t ClockFrequency = 32768;
  constexpr uint32_t ReloadValue = 0x6E524635UL;

  void SetBehaviours(Watchdog::SleepBehaviour sleepBehaviour, Watchdog::HaltBehaviour haltBehaviour) {
    /*
     * NRF_WDT->CONFIG : only the 1st and 4th bits are relevant.
     * Bit 0 : Behavior when the CPU is sleeping
     * Bit 3 : Behavior when the CPU is halted by the debugger
     * O means that the CPU is paused during sleep/halt, 1 means that the watchdog is kept running
     */
     NRF_WDT->CONFIG = static_cast<uint32_t>(sleepBehaviour) | static_cast<uint32_t>(haltBehaviour);
  }

  void SetTimeout(uint8_t timeoutSeconds) {
    /*
     * According to the documentation:
     *   Clock = 32768
     *   timeout [s] = ( CRV + 1 ) / Clock
     *   -> CRV = (timeout [s] * Clock) -1
     */
    NRF_WDT->CRV = (timeoutSeconds * ClockFrequency) - 1;
  }

  void EnableFirstReloadRegister() {
    /*
     * RREN (Reload Register Enable) is a bitfield of 8 bits. Each bit represents
     * one of the eight reload registers available. We enable only the first one.
     */
    NRF_WDT->RREN |= 1;
  }

  Watchdog::ResetReason GetResetReason() {
    /* NRF_POWER->RESETREAS
     * -------------------------------------------------------------------------------------------------------------------- *
     * Bit | Reason (if bit is set to 1)
     * ----|--------------------------------------------------------------------------------------------------------------- *
     *  0  | Reset from the pin reset
     *  1  | Reset from the watchdog
     *  2  | Reset from soft reset
     *  3  | Reset from CPU lock-up
     * 16  | Reset due to wake up from System OFF mode when wakeup is triggered from DETECT signal from GPIO
     * 17  | Reset due to wake up from System OFF mode when wakeup is triggered from ANADETECT signal from LPCOMP
     * 18  | Reset due to wake up from System OFF mode when wakeup is triggered from entering into debug interface mode
     * 19  | Reset due to wake up from System OFF mode by NFC field detect
     * -------------------------------------------------------------------------------------------------------------------- *
     */
    const uint32_t reason = NRF_POWER->RESETREAS;
    NRF_POWER->RESETREAS = 0xffffffff;

    uint32_t value = reason & 0x01; // avoid implicit conversion to bool using this temporary variable.
    if (value != 0) {
      return Watchdog::ResetReason::ResetPin;
    }

    value = (reason >> 1u) & 0x01u;
    if (value != 0) {
      return Watchdog::ResetReason::Watchdog;
    }

    value = (reason >> 2u) & 0x01u;
    if (value != 0) {
      return Watchdog::ResetReason::SoftReset;
    }

    value = (reason >> 3u) & 0x01u;
    if (value != 0) {
      return Watchdog::ResetReason::CpuLockup;
    }

    value = (reason >> 16u) & 0x01u;
    if (value != 0) {
      return Watchdog::ResetReason::SystemOff;
    }

    value = (reason >> 17u) & 0x01u;
    if (value != 0) {
      return Watchdog::ResetReason::LpComp;
    }

    value = (reason >> 18u) & 0x01u;
    if (value != 0) {
      return Watchdog::ResetReason::DebugInterface;
    }

    value = (reason >> 19u) & 0x01u;
    if (value != 0) {
      return Watchdog::ResetReason::NFC;
    }

    return Watchdog::ResetReason::HardReset;
  }
}

void Watchdog::Setup(uint8_t timeoutSeconds, SleepBehaviour sleepBehaviour, HaltBehaviour haltBehaviour) {
  SetBehaviours(sleepBehaviour, haltBehaviour);
  SetTimeout(timeoutSeconds);
  EnableFirstReloadRegister();

  resetReason = ::GetResetReason();
}

void Watchdog::Start() {
  NRF_WDT->TASKS_START = 1;
}

void Watchdog::Kick() {
  // NOTE : This driver enables only the 1st reload register.
  NRF_WDT->RR[0] = ReloadValue;
}

const char* Watchdog::GetResetReasonString() const {
  switch (resetReason) {
    case ResetReason::ResetPin:
      return "rst";
    case ResetReason::Watchdog:
      return "wtdg";
    case ResetReason::SoftReset:
      return "softr";
    case ResetReason::CpuLockup:
      return "cpulock";
    case ResetReason::SystemOff:
      return "off";
    case ResetReason::LpComp:
      return "lpcomp";
    case ResetReason::DebugInterface:
      return "dbg";
    case ResetReason::NFC:
      return "nfc";
    case ResetReason::HardReset:
      return "hardr";
    default:
      return "???";
    }
}
