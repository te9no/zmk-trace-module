#include "trace_json.h"

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zmk_trace_json, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_TRACE_USB_CDC) && DT_HAS_CHOSEN(zephyr_console)
static const struct device *trace_uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
#endif

int zmk_trace_emit_json(const char *json) {
#if IS_ENABLED(CONFIG_ZMK_TRACE_USB_CDC) && DT_HAS_CHOSEN(zephyr_console)
    if (device_is_ready(trace_uart)) {
        for (const char *p = json; *p != '\0'; p++) {
            uart_poll_out(trace_uart, *p);
        }
        uart_poll_out(trace_uart, '\n');
        return 0;
    }
#endif
    LOG_INF("%s", json);
    return 0;
}

int zmk_trace_emit_event(const char *name, const char *data_json) {
    char line[CONFIG_ZMK_TRACE_BUFFER_SIZE];
    snprintk(line, sizeof(line), "{\"type\":\"event\",\"name\":\"%s\",\"data\":%s}", name, data_json ? data_json : "{}");
    return zmk_trace_emit_json(line);
}
