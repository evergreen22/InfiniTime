#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class NotificationManager {
    public:
      enum class Categories {
        Unknown,
        SimpleAlert,
        Email,
        News,
        IncomingCall,
        MissedCall,
        Sms,
        VoiceMail,
        Schedule,
        HighProriotyAlert,
        InstantMessage
      };
      static constexpr uint8_t MessageSize {100}; // value is also set in Gadgetbridge

      struct Notification {
        bool valid = false;
        uint8_t notifNumber;
        uint8_t msgSize;
        std::array<char, MessageSize + 1> message;
        Categories category = Categories::Unknown;

        const char* Title() const;
        const char* Message() const;
      };

      void Push(Notification& notif);
      Notification GetLast();
      Notification GetPrevious();
      Notification GetNext();
      bool AreNewNotificationsAvailable() const;
      void ClearNewNotificationFlag();
      size_t NotifyCount() const;

      static constexpr size_t MaximumMessageSize() {
        return MessageSize;
      };

    private:
      static constexpr uint8_t bufSize = 6; // must be one more than capacity
      std::array<Notification, bufSize> notifications;
      uint8_t cBufHead = 0;
      uint8_t cBufTail = 0;
      uint8_t cBufNdx  = 0;
      uint8_t notifCnt = 0;
      uint8_t notifNum = 0;
      std::atomic<bool> newNotification {false};
    };
  }
}
