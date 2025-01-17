#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Notifications.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/DisplayApp.h"
#include "components/ble/AlertNotificationService.h"

using namespace Pinetime::Applications::Screens;

Notifications::Notifications(DisplayApp* app,
                             Pinetime::Controllers::NotificationManager& notificationManager,
                             Pinetime::Controllers::AlertNotificationService& alertNotificationService,
                             Pinetime::Controllers::MotorController& motorController,
                             uint32_t screenTimeOut,
                             Modes mode)
  : Screen(app),
    notificationManager {notificationManager},
    alertNotificationService {alertNotificationService},
    motorController {motorController},
    mode {mode} {
  auto notification = notificationManager.GetLast();
  if (notification.valid) {
    currentItem = std::make_unique<NotificationItem>(notification.Title(),
                                                     notification.Message(),
                                                     notification.notifNumber,
                                                     notification.category,
                                                     notificationManager.NotifyCount(),
                                                     mode,
                                                     alertNotificationService);
    validDisplay = true;
  } else {
    currentItem = std::make_unique<NotificationItem>("Notification",
                                                     "No notification to display",
                                                     0,
                                                     notification.category,
                                                     notificationManager.NotifyCount(),
                                                     Modes::Preview,
                                                     alertNotificationService);
  }

  if (mode == Modes::Preview) {
    if (notification.category == Pinetime::Controllers::NotificationManager::Categories::IncomingCall) {
      motorController.StartRinging();
    } else {
      motorController.RunForDuration(85);
    }
    timeoutLine = lv_line_create(lv_scr_act(), nullptr);

    lv_obj_set_style_local_line_width(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_line_color(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_line_rounded(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);

    lv_line_set_points(timeoutLine, timeoutLinePoints, 2);
    timeoutTickCountStart = xTaskGetTickCount();
    timeoutTickCountEnd = timeoutTickCountStart + screenTimeOut;

    taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  } else {
    notificationManager.ClearNewNotificationFlag();
  }
}

Notifications::~Notifications() {
  lv_task_del(taskRefresh);
  motorController.StopRinging();
  lv_obj_clean(lv_scr_act());
}

void Notifications::Refresh() {
  if (currentItem->IsRunning() == false) {
    running = false;
    return;
  }

  auto tick = xTaskGetTickCount();
  int32_t pos = 240 - ((tick - timeoutTickCountStart) / ((timeoutTickCountEnd - timeoutTickCountStart) / 240));
  if (pos < 0) {
    running = false;
  } else {
    timeoutLinePoints[1].x = pos;
    lv_line_set_points(timeoutLine, timeoutLinePoints, 2);
  }
}

bool Notifications::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if (mode == Modes::Preview && event == Pinetime::Applications::TouchEvents::SwipeUp) {
    running = false;
    return true;
  }

  if (mode == Modes::Preview) {
    return false;
  }

  switch (event) {
    case Pinetime::Applications::TouchEvents::SwipeDown: {
      Pinetime::Controllers::NotificationManager::Notification previousNotification;
      if (validDisplay)
        previousNotification = notificationManager.GetPrevious();
      else
        previousNotification = notificationManager.GetLast();

      if (!previousNotification.valid)
        return true;

      validDisplay = true;
      currentItem.reset(nullptr);
      app->SetFullRefresh(DisplayApp::FullRefreshDirections::Down);
      currentItem = std::make_unique<NotificationItem>(previousNotification.Title(),
                                                       previousNotification.Message(),
                                                       previousNotification.notifNumber,
                                                       previousNotification.category,
                                                       notificationManager.NotifyCount(),
                                                       mode,
                                                       alertNotificationService);
    }
      return true;
    case Pinetime::Applications::TouchEvents::SwipeUp: {
      Pinetime::Controllers::NotificationManager::Notification nextNotification;
      if (validDisplay)
        nextNotification = notificationManager.GetNext();
      else
        nextNotification = notificationManager.GetLast();

      if (!nextNotification.valid) {
        running = false;
        return false;
      }

      validDisplay = true;
      currentItem.reset(nullptr);
      app->SetFullRefresh(DisplayApp::FullRefreshDirections::Up);
      currentItem = std::make_unique<NotificationItem>(nextNotification.Title(),
                                                       nextNotification.Message(),
                                                       nextNotification.notifNumber,
                                                       nextNotification.category,
                                                       notificationManager.NotifyCount(),
                                                       mode,
                                                       alertNotificationService);
    }
      return true;
    default:
      return false;
  }
}

namespace {
  void CallEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* item = static_cast<Notifications::NotificationItem*>(obj->user_data);
    item->OnCallButtonEvent(obj, event);
  }
}

Notifications::NotificationItem::NotificationItem(const char* title,
                                                  const char* msg,
                                                  uint8_t notifNr,
                                                  Pinetime::Controllers::NotificationManager::Categories category,
                                                  uint8_t notifNb,
                                                  Modes mode,
                                                  Pinetime::Controllers::AlertNotificationService& alertNotificationService)
  : mode {mode}, alertNotificationService {alertNotificationService} {
  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x222222));
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 0, 50);
  lv_obj_set_size(container1, LV_HOR_RES, 190);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);
  lv_cont_set_fit(container1, LV_FIT_NONE);

  lv_obj_t* alert_count = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(alert_count, "%i/%i", notifNr, notifNb);
  lv_obj_align(alert_count, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 16);

  lv_obj_t* alert_type = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(alert_type, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x888888));
  if (title == nullptr)
    title = "Notification";
  char* pchar;
  pchar = strchr(title, '\n');
  while (pchar != nullptr) {
    *pchar = ' ';
    pchar = strchr(pchar + 1, '\n');
  }
  lv_label_set_text(alert_type, title);
  lv_label_set_long_mode(alert_type, LV_LABEL_LONG_SROLL_CIRC);
  lv_obj_set_width(alert_type, 180);
  lv_obj_align(alert_type, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 16);

  lv_obj_t* alert_msg = lv_label_create(container1, nullptr);
  lv_label_set_long_mode(alert_msg, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(alert_msg, LV_HOR_RES - 20);

  /////////
  switch (category) {
    default: {
      lv_obj_set_style_local_text_color(alert_msg, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
      lv_obj_set_style_local_text_font(alert_msg, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_28);
      lv_label_set_text(alert_msg, msg);
    } break;
    case Pinetime::Controllers::NotificationManager::Categories::IncomingCall: {
      lv_obj_set_height(container1, 108);

      lv_label_set_text(alert_msg, "Incoming call from");

      lv_obj_t* alert_caller = lv_label_create(container1, nullptr);
      lv_obj_set_style_local_text_color(alert_caller, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
      lv_obj_set_style_local_text_font(alert_caller, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_28);
      lv_obj_align(alert_caller, alert_msg, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
      lv_label_set_long_mode(alert_caller, LV_LABEL_LONG_BREAK);
      lv_obj_set_width(alert_caller, LV_HOR_RES - 20);
      lv_label_set_text(alert_caller, msg);

      bt_accept = lv_btn_create(lv_scr_act(), nullptr);
      bt_accept->user_data = this;
      lv_obj_set_event_cb(bt_accept, CallEventHandler);
      lv_obj_set_size(bt_accept, 76, 76);
      lv_obj_align(bt_accept, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
      label_accept = lv_label_create(bt_accept, nullptr);
      lv_label_set_text(label_accept, Symbols::phone);
      lv_obj_set_style_local_bg_color(bt_accept, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);

      bt_reject = lv_btn_create(lv_scr_act(), nullptr);
      bt_reject->user_data = this;
      lv_obj_set_event_cb(bt_reject, CallEventHandler);
      lv_obj_set_size(bt_reject, 76, 76);
      lv_obj_align(bt_reject, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
      label_reject = lv_label_create(bt_reject, nullptr);
      lv_label_set_text(label_reject, Symbols::phoneSlash);
      lv_obj_set_style_local_bg_color(bt_reject, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

      bt_mute = lv_btn_create(lv_scr_act(), nullptr);
      bt_mute->user_data = this;
      lv_obj_set_event_cb(bt_mute, CallEventHandler);
      lv_obj_set_size(bt_mute, 76, 76);
      lv_obj_align(bt_mute, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
      label_mute = lv_label_create(bt_mute, nullptr);
      lv_label_set_text(label_mute, Symbols::volumMute);
      lv_obj_set_style_local_bg_color(bt_mute, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    } break;
  }

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");
}

void Notifications::NotificationItem::OnCallButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }

  if (obj == bt_accept) {
    alertNotificationService.AcceptIncomingCall();
  } else if (obj == bt_reject) {
    alertNotificationService.RejectIncomingCall();
  } else if (obj == bt_mute) {
    alertNotificationService.MuteIncomingCall();
  }

  running = false;
}

Notifications::NotificationItem::~NotificationItem() {
  lv_obj_clean(lv_scr_act());
}
