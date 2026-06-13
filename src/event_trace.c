#include "trace_json.h"

#include <zephyr/kernel.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/keycode_state_changed.h>

static int zmk_trace_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *pos = as_zmk_position_state_changed(eh);
    if (pos != NULL) {
        char data[96];
        snprintk(data, sizeof(data), "{\"source\":%u,\"position\":%u,\"state\":%s,\"timestamp\":%lld}",
                 pos->source, pos->position, pos->state ? "true" : "false",
                 (long long)pos->timestamp);
        zmk_trace_emit_event("position_state_changed", data);
        return ZMK_EV_EVENT_BUBBLE;
    }

    const struct zmk_keycode_state_changed *kc = as_zmk_keycode_state_changed(eh);
    if (kc != NULL) {
        char data[128];
        snprintk(data, sizeof(data), "{\"usage_page\":%u,\"keycode\":%u,\"implicit_mods\":%u,\"explicit_mods\":%u,\"state\":%s,\"timestamp\":%lld}",
                 kc->usage_page, kc->keycode, kc->implicit_modifiers, kc->explicit_modifiers,
                 kc->state ? "true" : "false", (long long)kc->timestamp);
        zmk_trace_emit_event("keycode_state_changed", data);
        return ZMK_EV_EVENT_BUBBLE;
    }

    const struct zmk_layer_state_changed *layer = as_zmk_layer_state_changed(eh);
    if (layer != NULL) {
        char data[96];
        snprintk(data, sizeof(data), "{\"layer\":%u,\"state\":%s,\"locked\":%s,\"timestamp\":%lld}",
                 layer->layer, layer->state ? "true" : "false",
                 layer->locked ? "true" : "false", (long long)layer->timestamp);
        zmk_trace_emit_event("layer_state_changed", data);
        return ZMK_EV_EVENT_BUBBLE;
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(zmk_trace, zmk_trace_listener);
ZMK_SUBSCRIPTION(zmk_trace, zmk_position_state_changed);
ZMK_SUBSCRIPTION(zmk_trace, zmk_keycode_state_changed);
ZMK_SUBSCRIPTION(zmk_trace, zmk_layer_state_changed);
