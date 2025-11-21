#ifndef NOTIFICATION_TOAST_H
#define NOTIFICATION_TOAST_H

#include "esp_err.h"
#include "services/notification_service.h"

/**
 * @brief Initialize notification toast widget
 *
 * Subscribes to EVENT_NOTIFICATION_NEW to automatically show toast
 */
esp_err_t notification_toast_init(void);

/**
 * @brief Show a notification toast
 *
 * Creates a temporary overlay on top of current screen.
 * Auto-dismisses after 3 seconds.
 * Can be dismissed by swiping right.
 *
 * @param notification Notification to show
 */
void notification_toast_show(const notification_t *notification);

/**
 * @brief Dismiss current toast
 */
void notification_toast_dismiss(void);

#endif // NOTIFICATION_TOAST_H
