# Node Builder — TODO List

**Project:** OpenLcbCLib  
**Tool:** node_builder  
**Date Started:** 2026-03-01

---

## Open Questions (Decide Before Implementation)

- [x] Where does `node_config.h` live? Application directory (alongside `node_parameters.c`)
      or a project-level location above the src tree?
      DECISION: Top-level application directory. `NODE_CDI_XML_PATH` and
      `NODE_FDI_XML_PATH` are relative to that directory. The tool resolves all paths
      relative to the location of `node_config.h`.

- [x] Should the CDI/FDI XML source be stored as a separate file in the project, or
      embedded as a comment block inside `node_config.h` to keep it self-contained?
      DECISION: Stored as separate files. `node_config.h` holds the path to the XML
      files as macros (e.g. NODE_CDI_XML_PATH and NODE_FDI_XML_PATH). Users edit the
      XML in their favorite XML editor. The node_builder tool reads the path, loads
      the file, and regenerates the byte array and derived values. The XML files travel
      with the project alongside node_config.h.

- [ ] CLI tool dependency question — OPEN.
      Constraint: cannot depend on Python being installed. Cannot write files from the
      browser directly. Options under consideration:
        a) Browser tool generates a sh/bat script containing the byte array regeneration
           logic that the user runs locally — but this pushes complexity into a shell
           script that must handle XML parsing.
        b) Browser tool handles everything at generate-time (XML is loaded into the
           browser, processed there, node_config.h downloaded) — round-trip update
           requires re-opening the tool and re-loading node_config.h.
        c) A small self-contained native binary (no runtime dependencies).
      For now: browser tool is the primary delivery. CLI approach deferred.

- [x] Should the tool optionally emit a `callbacks.c` stub matched to the enabled feature
      flags?
      DECISION: Yes. The tool generates `callbacks.c/.h` from checkboxes — user selects
      which callbacks they want. Generated file is a working stub (empty function bodies)
      not just a template comment block. Callback list is driven by enabled feature flags
      (e.g. broadcast time callback checkbox only appears if OPENLCB_COMPILE_BROADCAST_TIME
      is checked). Callbacks that are mandatory regardless of feature flags are always
      included and their checkboxes are locked.

- [x] Resolve the `node_config.h` macro naming convention — current spec uses `NODE_*`
      prefix throughout. Confirm this does not conflict with any existing library macros.
      DECISION: `NODE_*` prefix is confirmed clean. Grep of all core library headers
      (src/openlcb, src/drivers, src/utilities) found zero existing `#define NODE_`
      macros. Naming convention stands.

---

## SPEC.md

- [x] Review all member-by-member mapping rules in Section 5 for correctness against the
      actual library behavior.

- [x] Remove Section 5.1.3 (`consumer_count_autocreate` / `producer_count_autocreate`)
      from the spec entirely — these fields are being removed from the library.

- [x] Add a "Visibility" column to all 5.1.x member tables.

- [x] Update Section 5.1.5 (Space 0xFF): `present` computed from CDI file presence.

- [x] Update Section 5.1.1 (snip mapping rules): SNIP manufacturer fields extracted
      from CDI <identification> when CDI present; editable only when no CDI loaded.

- [ ] Confirm `NODE_SNIP_MFG_VERSION` fixed value (4) and `NODE_SNIP_USER_VERSION` fixed
      value (2) match current spec and library behavior — or should these be user-settable?

- [ ] Confirm `configuration_options.high_address_space` and `low_address_space` are
      truly computable from the present/absent address space flags, or does the user need
      to set these explicitly.

- [ ] Add the full list of `#error` guard expressions to Section 6 (currently only
      described in prose/table — should include actual C preprocessor syntax for reference).

- [x] Decide whether Space 0xFE (All Memory) is required, optional, or platform-specific.
      DECISION: Optional. Up to the application whether to advertise it and how to
      configure it. The tool shows Space 0xFE controls unconditionally (always visible)
      with `present` as a user checkbox. All other Space 0xFE fields are gated on
      `present` = true. Update spec 5.1.6 and visibility summary accordingly.

- [ ] Clarify multi-node behavior: `address_space_config_memory.highest_address` (Space 0xFD)
      comment says each node's config block = `highest_address + 1` bytes. Verify this is
      correct and document the formula explicitly.

---

## node_config.h Template

- [ ] Write the actual `node_config.h` template file with all sections, macros, and
      comments (the spec describes the structure — the template is the deliverable).

- [ ] Decide on default values for each macro in the template (should match the existing
      Xcode BasicNode demo as the reference starting point).

- [ ] Add in-file comments that cross-reference dependencies (e.g. "changing this requires
      regenerating the CDI — run node_builder").

---

## node_parameters.c Refactor

- [ ] Refactor `node_parameters.c` in each demo to consume `node_config.h` macros instead
      of hardcoded literals — do Xcode demo first as the reference.

- [ ] Update `node_parameters.h` in each demo accordingly.

- [ ] Confirm the refactored `node_parameters.c` compiles cleanly on all demo platforms.

---

## openlcb_user_config.h Backward Compatibility

- [ ] Add backward-compat `#define` aliases in `openlcb_user_config.h` so existing code
      that references `USER_DEFINED_*` names continues to compile:
      - `USER_DEFINED_CDI_LENGTH` → `NODE_CDI_LENGTH`
      - `USER_DEFINED_FDI_LENGTH` → `NODE_FDI_LENGTH`
      - `USER_DEFINED_BASIC_BUFFER_DEPTH` → `NODE_BASIC_BUFFER_DEPTH`
      - (and all remaining USER_DEFINED_* macros)

---

## openlcb_node_config_validate.h (New Library File)

- [ ] Create `src/openlcb/openlcb_node_config_validate.h` with all `#error` guards.

- [ ] Include it from the appropriate library header so validation runs automatically
      on every build (candidate: `openlcb_types.h` or `openlcb_config.h`).

- [ ] Write and verify all guards listed in SPEC.md Section 6.

---

## Callbacks Generation

- [ ] Define the full callback table — which callbacks are mandatory (always generated,
      checkbox locked) vs. optional (checkbox only visible when relevant feature flag
      is enabled):

      MANDATORY (always included, checkbox locked):
        - Callbacks_initialize(void)
        - Callbacks_on_100ms_timer_callback(void)
        - Callbacks_on_can_rx_callback(can_msg_t *can_msg)
        - Callbacks_on_can_tx_callback(can_msg_t *can_msg)
        - Callbacks_alias_change_callback(uint16_t new_alias, node_id_t node_id)
        - Callbacks_on_login_complete(openlcb_node_t *openlcb_node)

      CONDITIONAL on OPENLCB_COMPILE_CONFIG_MEMORY:
        - Callbacks_operations_request_factory_reset(...)

      CONDITIONAL on OPENLCB_COMPILE_EVENTS:
        - Callbacks_on_consumed_event_identified(...)
        - Callbacks_on_consumed_event_pcer(...)
        - Callbacks_on_event_learn(...)

      CONDITIONAL on OPENLCB_COMPILE_BROADCAST_TIME:
        - Callbacks_on_broadcast_time_changed(broadcast_clock_t *clock)

      CONDITIONAL on OPENLCB_COMPILE_TRAIN (not yet implemented):
        - TBD when train callbacks are defined

- [x] Verify which callbacks are truly mandatory vs. optional without a feature flag.
      DECISION: `Callbacks_on_can_rx_callback` and `Callbacks_on_can_tx_callback` are
      mandatory on all transports including WiFi. The library uses `can_msg_t` as its
      internal message format regardless of physical transport — these callbacks fire
      for every message in and out. Confirmed by inspecting the WiFi demo callbacks.c
      which implements both identically to the CAN demo.
      NOTE: A pure TCP/IP transport layer is planned for the future. The callback names
      are already transport-misleading. When TCP/IP is added the callback names and
      signatures will likely need revisiting — do not treat "can" in the name as
      implying CAN-only. The generated stub comment should make this clear.

- [ ] Decide behavior when `callbacks.c/.h` already exists in the project:
      tool should warn and offer to diff/skip rather than silently overwrite.

- [ ] Write stub body content for each callback — should include a comment explaining
      what the callback is for and what the user is expected to do inside it.

- [ ] Add Callbacks panel to `node_builder.html` UI with checkbox list.

- [ ] Add callbacks generation to `node_builder.py` CLI.

---

## node_builder.html (Browser Tool)

- [ ] Design the UI layout — collapsible sections matching `node_config.h` structure.

- [ ] Implement Feature Flags panel (checkboxes with dependency highlighting).

- [ ] Implement Node Identity panel (text inputs with SNIP field length enforcement).

- [ ] Implement Protocol Support panel (auto-populated from flags, user-overridable,
      mismatch warnings).

- [ ] Implement Buffer Sizing panel (numeric inputs).

- [ ] Implement Event System panel (numeric inputs).

- [ ] Implement Memory Configuration panel.

- [ ] Implement CDI panel (XML file picker, byte array generator, length display).
      Panel includes a "Generate Starter CDI XML" button that produces a minimal but
      valid CDI XML file pre-populated from the current SNIP fields (manufacturer, model,
      hardware version, software version) and a single empty segment as a starting point.
      File is downloaded to the browser's download folder as `node_cdi.xml` and
      `NODE_CDI_XML_PATH` is set to that filename automatically.

- [ ] Implement FDI panel (same as CDI, hidden unless Train flag is checked).
      Panel includes a "Generate Starter FDI XML" button that produces a minimal but
      valid FDI XML file with an empty function list as a starting point.
      File is downloaded as `node_fdi.xml` and `NODE_FDI_XML_PATH` is set automatically.

- [ ] Implement Address Spaces panel (computed/fixed values read-only, policy flags
      editable).

- [ ] Implement Summary / Validation panel (cross-check all values before generating).

- [ ] Implement round-trip: load existing `node_config.h`, parse macros, populate form.

- [ ] Implement Generate button: produce `node_config.h` and trigger browser download.

- [ ] Test CDI/FDI XML processing matches output of existing `cdi_to_array.py` exactly.

---

## node_builder.py (CLI Tool)

- [ ] Implement `--new` interactive mode.

- [ ] Implement `--from answers.json` batch mode.

- [ ] Implement `--validate node_config.h` mode.

- [ ] Implement `--update node_config.h --cdi new_cdi.xml` mode.

- [ ] Implement `--update node_config.h --fdi new_fdi.xml` mode.

- [ ] Ensure CDI/FDI output exactly matches `node_builder.html` output (single source
      of processing logic if possible).

---

## Demo Updates

- [ ] Xcode BasicNode — update to `node_config.h` pattern (reference implementation).
- [ ] PlatformIO OSX BasicNode
- [ ] PlatformIO ESP32 BasicNode
- [ ] PlatformIO ESP32 BasicNode_WiFi
- [ ] Arduino ESP32 BasicNode
- [ ] Arduino RPi Pico BasicNode
- [ ] STM32 CubeIDE stm32f407 BasicNode
- [ ] TI Theia MSPM03507 BasicNode
- [ ] dsPIC BasicNode.X

---

## Deprecation

- [ ] Mark `tools/cdi_to_array/` as deprecated in its README.
- [ ] Mark `tools/cdi_to_array_webbrowser/` as deprecated in its README.
- [ ] Add note pointing users to `tools/node_builder/` instead.
