#include <algorithm>
#include <cstring>

#include "components/ble/NotificationManager.h"

using namespace Pinetime::Controllers;

constexpr uint8_t NotificationManager::MessageSize;

void NotificationManager::Push(NotificationManager::Notification& notif) {
  if ((cBufTail + 1) % bufSize == cBufHead) {
    notifications[cBufHead].valid = false;
    cBufHead = (cBufHead + 1) % bufSize;
    notifCnt--;
   }

  notif.valid = true;
  notif.notifNumber = 1;

  notifications[cBufTail] = notif;
  cBufTail = (cBufTail + 1) % bufSize;
  notifCnt++;

  cBufNdx = cBufTail;
  notifNum = 0;

  newNotification.exchange(true);
}

NotificationManager::Notification NotificationManager::GetLast() {
  cBufNdx = cBufTail;
  notifNum = 0;

  return GetPrevious();
}

NotificationManager::Notification NotificationManager::GetPrevious() {
  NotificationManager::Notification notif;

  if ((cBufTail != cBufHead) && (cBufNdx != cBufHead)) {
    if (cBufNdx == 0)
      cBufNdx = bufSize - 1;
    else
      cBufNdx--;

    notif = notifications[cBufNdx];
    notif.notifNumber = ++notifNum;
  }

  return notif;
}

NotificationManager::Notification NotificationManager::GetNext() {
  NotificationManager::Notification notif;

  if ((cBufTail != cBufHead) && (((cBufNdx + 1) % bufSize) != cBufTail)) {
    cBufNdx = (cBufNdx + 1) % bufSize;

    notif = notifications[cBufNdx];
    notif.notifNumber = --notifNum;
  }

  return notif;
}

bool NotificationManager::AreNewNotificationsAvailable() const {
  return newNotification;
}

void NotificationManager::ClearNewNotificationFlag() {
  newNotification.exchange(false);
}

size_t NotificationManager::NotifyCount() const {
  return notifCnt;
}

const char* NotificationManager::Notification::Title() const {
  const char* str = std::find(message.cbegin(), message.cend(), '\0');

  if (str < (message.cbegin() + msgSize - 1))
    return const_cast<char*>(message.data());

  return nullptr;
}

const char* NotificationManager::Notification::Message() const {
  const char* str = std::find(message.cbegin(), message.cend(), '\0');

  if (str == (message.cbegin() + msgSize - 1))
    return const_cast<char*>(message.data());

  return (str + 1);
}
