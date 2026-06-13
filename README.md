# zmk-trace-module

Experimental ZMK module scaffold for streaming real keyboard traces to `zmk-gpio-sim.html`.

Web UI:

- GitHub Pages: https://te9no.github.io/zmk-trace-module/
- Local file: `docs/index.html`

The first target transport is USB CDC serial with JSON Lines:

```json
{"type":"input_stage","device":"trackball2","listener":"trackball_listener2","processor":"scroll_runtime_input_processor_l","before":{"x":12,"y":-4,"scroll_x":0,"scroll_y":0},"after":{"x":0,"y":0,"scroll_x":12,"scroll_y":-4}}
{"type":"event","name":"layer_state_changed","data":{"layer":1,"state":true}}
```

## Add to a ZMK config

In `config/west.yml`, add this module as a project:

```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: local
      url-base: https://github.com/te9no
  projects:
    - name: zmk
      remote: zmkfirmware
      revision: main
      import: app/west.yml
    - name: zmk-trace-module
      remote: local
      revision: main
  self:
    path: config
```

Then enable it in your board/shield `.conf`:

```conf
CONFIG_ZMK_TRACE=y
CONFIG_ZMK_TRACE_USB_CDC=y
CONFIG_ZMK_TRACE_INPUT_PROCESSOR=y
```

The transport writes raw JSON Lines to the Zephyr `zephyr,console` UART. For USB CDC builds, make sure your board/config routes console output to CDC ACM. The exact symbols vary by board/ZMK revision, but the usual direction is enabling USB device/CDC ACM console and reducing unrelated logs while tracing.

And add a trace input processor around the processors you want to inspect:

```dts
input-processors =
  <&trace_input_raw>,
  <&scroll_runtime_input_processor_l>,
  <&trace_input_after_runtime>,
  <&zip_xy_to_scroll_mapper>,
  <&trace_input_after_mapper>;
```

Each trace processor emits the event value at the point where it is inserted. Place one before and after processors you care about to reconstruct before/after in the browser.

For `te9no/zmk-config-GeaconPolaris`, see:

- `examples/GeaconPolaris_TB_L_TRACE.overlay`: traces `trackball_listener2` around `scroll_runtime_input_processor_l` and `zip_xy_to_scroll_mapper`.
- `examples/GeaconPolaris_IQS_TRACE.overlay`: traces `iqs_listener` around fixed and dynamic scaling processors.

These examples are intentionally overlays, not permanent config. Add one while debugging a module, build/flash, connect `zmk-gpio-sim.html` via Web Serial, then remove it when done.

## Files

- `zephyr/module.yml`: Zephyr module registration.
- `Kconfig`: Config switches.
- `CMakeLists.txt`: Source registration.
- `dts/bindings/input_processor/zmk,input-processor-trace.yaml`: Devicetree binding for trace processors.
- `app.overlay`: Example trace processor nodes.
- `examples/GeaconPolaris_TB_L_TRACE.overlay`: Concrete trackball trace insertion example.
- `examples/GeaconPolaris_IQS_TRACE.overlay`: Concrete IQS touch/pointing trace insertion example.
- `src/input_processor_trace.c`: Trace processor implementation for ZMK main's `drivers/input_processor.h` API.
- `src/event_trace.c`: Passive event listener skeleton.
- `docs/index.html`: Browser UI for loading ZMK configs, simulating virtual input, and viewing trace JSON Lines.

## Current status

This is still experimental. ZMK input processor APIs may differ across pointing-device branches, but the included implementation matches ZMK main's current `drivers/input_processor.h` API. The browser app understands the JSON Lines above via Web Serial.
