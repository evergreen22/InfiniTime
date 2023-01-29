#include "displayapp/screens/BatteryIcon.h"
#include <cstdint>
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

const char* BatteryIcon::GetBatteryIcon(uint8_t batteryPercent) {
  if (batteryPercent > 95)
    return Symbols::batteryFull;
  if (batteryPercent > 70)
    return Symbols::batteryThreeQuarter;
  if (batteryPercent > 40)
    return Symbols::batteryHalf;
  if (batteryPercent > 25)
    return Symbols::batteryOneQuarter;
  return Symbols::batteryEmpty;
}

const char* BatteryIcon::GetUnknownIcon() {
  return Symbols::batteryEmpty;
}

const char* BatteryIcon::GetPlugIcon(bool isCharging) {
  if (isCharging)
    return Symbols::plug;
  else
    return "";
}
