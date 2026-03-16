# Node Wizard CDI Test Cases

Reference: OpenLCB CDI Standard (ConfigurationDescriptionInformationS.pdf, June 14 2025)

## Test Runner

Open `test_runner.html` in a browser and drag all `.xml` files from `test/cdi/` onto
the drop zone. The runner:

- Reads each XML via FileReader (no server needed)
- Parses `<!-- TEST_CHECKS -->` comment blocks embedded in each XML
- Runs validation (`CdiValidator.validate`) and map (`CdiRenderer.buildMap`) checks
- Runs compile checks (`CdiRenderer.checkDefineCollisions`, `CdiRenderer.checkStructCollisions`)
- All code is shared with the live CDI editor — no duplication

### TEST_CHECKS Format

Each XML file embeds its own expected results:

```xml
<!-- TEST_CHECKS
name: Test Name
validate_count | severity:error | expected:0 | No validation errors
map_not_null | Map parses
defines_no_dupes | No duplicate #define names
struct_no_dupes | No duplicate struct field names
-->
```

### Available Check Types

| Check Type | Params | Purpose |
|------------|--------|---------|
| `validate_count` | `severity`, `expected` | Exact count of errors/warnings/infos |
| `validate_min_count` | `severity`, `min` | Minimum count of errors/warnings/infos |
| `validate_has` | `severity`, `pattern` | At least one message matching regex |
| `map_is_null` | | Map fails to parse |
| `map_not_null` | | Map parses successfully |
| `map_no_overlaps` | | No address overlaps in any space |
| `map_has_overlaps` | | At least one overlap exists |
| `map_overlap_count` | `space`, `min` | Minimum overlaps in a specific space |
| `map_has_space` | `space` | Address space exists in map |
| `map_missing_space` | `space` | Address space is absent |
| `map_has_items` | `space`, `min` | Minimum item count in a space |
| `map_origin` | `space`, `expectedOrigin` | Segment origin matches expected value |
| `map_first_addr` | `space`, `minAddr` | First element address >= minimum |
| `map_space_exceeds` | `space`, `maxAddr` | Highest address exceeds limit |
| `map_duplicate_names` | `space` | Duplicate leaf names detected |
| `defines_no_dupes` | | No duplicate `#define` names (compilable) |
| `defines_has_dupes` | | Duplicate `#define` names expected |
| `struct_no_dupes` | | No duplicate struct field names (compilable) |
| `struct_has_dupes` | | Duplicate struct field names expected |

---

## Test Files

### `cdi/01_valid_typical.xml` — Baseline Valid CDI
**Tests:** Baseline — all features working with no errors
**Features:** identification, acdi, segment with strings/ints/eventids, groups, replication, map enumerations
| Check | Expected |
|-------|----------|
| Validate | No errors, no warnings |
| Memory Map | Clean render, no overlaps, space 253 present |
| Defines | No duplicate `#define` names |
| Struct | No duplicate struct field names |

---

### `cdi/02_malformed_xml.xml` — Malformed XML
**Tests:** Layer 1 XML well-formedness check
**Defect:** Unclosed `<model>` tag
| Check | Expected |
|-------|----------|
| Validate | **Error:** XML parse error |
| Memory Map | Map returns null (unparseable) |

---

### `cdi/03_wrong_root_element.xml` — Wrong Root Element
**Tests:** Layer 2 structural check — root must be `<cdi>`
**Defect:** Root element is `<configuration>` instead of `<cdi>`
| Check | Expected |
|-------|----------|
| Validate | **Error:** "Root element is not `<cdi>`" |

---

### `cdi/04_missing_identification.xml` — No Identification
**Tests:** Layer 3 semantic check — missing `<identification>`
| Check | Expected |
|-------|----------|
| Validate | **Warning:** missing identification; no errors |
| Compile | No duplicate defines or struct fields |

---

### `cdi/05_missing_acdi.xml` — No ACDI Tag
**Tests:** Layer 3 semantic check — missing `<acdi/>`
| Check | Expected |
|-------|----------|
| Validate | **Info:** missing acdi; no errors |
| Compile | No duplicate defines or struct fields |

---

### `cdi/06_segment_missing_space.xml` — Segment Missing Space Attribute
**Tests:** Layer 2 structural check — required attribute
**Defect:** `<segment>` has no `space` attribute (required by CDI spec Section 5.1.3)
| Check | Expected |
|-------|----------|
| Validate | **Error:** missing space attribute |

---

### `cdi/07_string_missing_size.xml` — String Missing Size Attribute
**Tests:** Layer 2 structural check — address calculation accuracy
**Defect:** First `<string>` has no `size` attribute (required by CDI spec Section 5.1.4.3)
| Check | Expected |
|-------|----------|
| Validate | **Warning:** missing string size |
| Memory Map | Map still parses (with wrong addresses) |
| Compile | No duplicate defines or struct fields |

---

### `cdi/08_int_invalid_size.xml` — Int with Invalid Size
**Tests:** Layer 2 structural check — int size validation
**Defect:** Two `<int>` elements with size=3 and size=5 (valid: 1, 2, 4, 8)
| Check | Expected |
|-------|----------|
| Validate | **Error:** invalid int size — at least 2 errors |
| Compile | No duplicate defines or struct fields |

---

### `cdi/09_min_greater_than_max.xml` — Min Greater Than Max
**Tests:** Layer 2 range validation
**Defect:** `<int>` with min=100 max=10; `<float>` with min=99.5 max=1.0
| Check | Expected |
|-------|----------|
| Validate | **Warning:** inverted min/max — at least 2 warnings |
| Compile | No duplicate defines or struct fields |

---

### `cdi/10_checkbox_bad_map.xml` — Checkbox Hint with Wrong Map Count
**Tests:** Layer 2 checkbox/map relationship (CDI spec Section 5.1.4.2)
**Defects:** 3 relations, 1 relation, and no map at all
| Check | Expected |
|-------|----------|
| Validate | **Error:** checkbox map relations wrong |
| Compile | No duplicate defines or struct fields |

---

### `cdi/11_address_overlap.xml` — Address Space Overlap
**Tests:** Memory map overlap detection
**Defect:** Negative offset causes EEPROM/FLASH data corruption at runtime
| Check | Expected |
|-------|----------|
| Memory Map | Map parses, overlaps detected, at least 1 overlap in space 253 |
| Compile | No duplicate defines or struct fields |

---

### `cdi/12_duplicate_names.xml` — Duplicate Names (Same Scope)
**Tests:** C code generation — duplicate identifiers within the same segment
**Defects:** Two ints named "Mode", two strings named "Label", two eventids named "Event On"
| Check | Expected |
|-------|----------|
| Memory Map | Duplicate leaf names detected in space 253 |
| Defines | **Duplicate `#define` names expected** (same-scope duplicates cause redefinition errors) |
| Struct | **Duplicate struct field names expected** (same-scope duplicates cause compile errors) |

---

### `cdi/13_duplicate_names_across_groups.xml` — Duplicate Names Across Groups
**Tests:** C code generation — name scoping across group boundaries
**Feature:** "Brightness" in group "Inputs", group "Outputs", and replicated "Channel"
| Check | Expected |
|-------|----------|
| Memory Map | At least 5 items in space 253 |
| Defines | No duplicates — group prefixes disambiguate (`INPUTS_BRIGHTNESS` vs `OUTPUTS_BRIGHTNESS`) |
| Struct | No duplicates — nested structs disambiguate |

---

### `cdi/14_config_mem_too_small.xml` — Config Memory Too Small
**Tests:** Config memory size cross-check
**Defect:** CDI defines ~1,200 bytes in space 253 vs default 0x200 (512 bytes)
| Check | Expected |
|-------|----------|
| Memory Map | Space 253 exceeds 0x200 bytes |
| Compile | No duplicate defines or struct fields |

---

### `cdi/15_unknown_elements.xml` — Unknown/Non-Standard Elements
**Tests:** Layer 2 unknown element detection (CDI spec Section 6)
**Defects:** `<custom>`, `<widget>`, `<bogus size="4">`
| Check | Expected |
|-------|----------|
| Validate | **Warning:** unknown/unrecognized elements |
| Memory Map | Map parses |
| Compile | No duplicate defines or struct fields |

---

### `cdi/16_segment_with_origin.xml` — Non-Zero Segment Origin
**Tests:** Address calculation with origin attribute (CDI spec Section 5.1.3)
**Feature:** Segment starts at origin=256 (0x100)
| Check | Expected |
|-------|----------|
| Memory Map | Origin is 256, first address >= 256 |
| Compile | No duplicate defines or struct fields |

---

### `cdi/17_multiple_spaces.xml` — Multiple Address Spaces
**Tests:** Correct separation of address spaces
**Feature:** Segments in spaces 253, 254, and 251
| Check | Expected |
|-------|----------|
| Memory Map | All three spaces present |
| Compile | No duplicate defines or struct fields |

---

### `cdi/18_empty_segment.xml` — Empty Segment
**Tests:** Segments with no data elements
| Check | Expected |
|-------|----------|
| Validate | **Info:** empty segment |
| Memory Map | Map parses |
| Compile | No duplicate defines or struct fields |

---

### `cdi/19_no_space_253.xml` — No Config Memory Space
**Tests:** CDI without standard config memory space (0xFD / 253)
**Feature:** Only space 249 (train function config)
| Check | Expected |
|-------|----------|
| Validate | **Info:** missing space 253 |
| Memory Map | Space 253 absent |
| Compile | No duplicate defines or struct fields |

---

### `cdi/20_deep_nesting.xml` — Deeply Nested Groups with Replication
**Tests:** Address calculation through nested replicated groups
**Feature:** Bank (x3) > Channel (x2) > eventid + eventid + int
| Check | Expected |
|-------|----------|
| Memory Map | 20+ items, no overlaps |
| Compile | No duplicate defines or struct fields |

---

### `cdi/21_all_data_types.xml` — Every Data Element Type
**Tests:** All CDI data types: int (1,2,4,8), string, eventid, float (4,8), action
| Check | Expected |
|-------|----------|
| Validate | No errors |
| Memory Map | Parses, no overlaps |
| Compile | No duplicate defines or struct fields |

---

### `cdi/22_special_chars_in_names.xml` — Special Characters in Names
**Tests:** C identifier sanitization for code generation
**Feature:** Names with `/`, `()`, `#`, `ü`, `%`, `.`
| Check | Expected |
|-------|----------|
| Validate | No errors (names are cosmetic) |
| Memory Map | Parses |
| Compile | No duplicates after sanitization |

---

### `cdi/23_offset_attribute.xml` — Offset Attribute
**Tests:** Address calculation with positive offsets (gaps/padding)
**Feature:** Offsets of +16 and +64 bytes
| Check | Expected |
|-------|----------|
| Memory Map | Parses, no overlaps |
| Compile | No duplicate defines or struct fields |

---

### `cdi/24_float_unusual_size.xml` — Float Unusual Size
**Tests:** Float size validation (CDI spec Section 5.1.4.5)
**Defect:** float size=3 is invalid; float size=2 is uncommon but valid
| Check | Expected |
|-------|----------|
| Validate | **Warning:** unusual float size |
| Memory Map | Parses |
| Compile | No duplicate defines or struct fields |

---

### `cdi/25_stray_text_in_string.xml` — Stray Text in String
**Tests:** Layer 2 stray-text check for `<string>` variable elements
**Defect:** Character `3` typed between opening tag and `<name>`, between `</name>` and `<description>`, and after `</description>`
| Check | Expected |
|-------|----------|
| Validate | **Error:** stray text inside `<string>` |
| Memory Map | Parses |

---

### `cdi/26_stray_text_in_int.xml` — Stray Text in Int and Float
**Tests:** Layer 2 stray-text check for `<int>` and `<float>` variable elements
**Defect:** Text `JUNK` inside `<int>`, text `oops` inside `<float>`
| Check | Expected |
|-------|----------|
| Validate | **Error:** stray text in `<int>` and `<float>` |

---

### `cdi/27_stray_text_in_eventid.xml` — Stray Text in EventID
**Tests:** Layer 2 stray-text check for `<eventid>` variable elements
**Defect:** Text `abc` inside `<eventid>`
| Check | Expected |
|-------|----------|
| Validate | **Error:** stray text inside `<eventid>` |

---

### `cdi/28_stray_text_in_containers.xml` — Stray Text in Container Elements
**Tests:** Layer 2 stray-text check for `<segment>`, `<group>`, `<identification>`, `<map>`, `<relation>`
**Defect:** Each container has a `stray_xxx` string as direct text content
| Check | Expected |
|-------|----------|
| Validate | **Error:** stray text in each of the five container types |

---

### `cdi/29_stray_text_short_chars.xml` — Short Stray Characters (findLine Accuracy)
**Tests:** `_findLine` does not false-match single characters inside attribute values
**Defect:** Single `3` and `1` characters inside `<string>` elements; both appear in the `xmlns` URI on the `<cdi>` line
| Check | Expected |
|-------|----------|
| Validate | **Error:** stray text in `<string>` (2 errors, NOT on the `<cdi>` line) |

---

### `cdi/30_stray_text_whitespace_ok.xml` — Whitespace-Only Text Nodes (No False Positives)
**Tests:** Generous whitespace and blank lines between child elements is legal
| Check | Expected |
|-------|----------|
| Validate | No errors, no warnings |
| Memory Map | Parses, no overlaps |
| Compile | No duplicates |

---

### `cdi/31_stray_text_multiple_elements.xml` — Stray Text in Multiple Element Types
**Tests:** Stray text in `<segment>`, `<string>`, `<group>`, `<int>`, `<eventid>` — all in one file
**Defect:** `STRAY_SEG`, `STRAY_STR`, `STRAY_GRP`, `STRAY_INT`, `STRAY_EVT` in respective elements
| Check | Expected |
|-------|----------|
| Validate | **Error:** at least 5 stray-text errors, one per element type |

---

### `cdi/32_stray_text_in_hints.xml` — Stray Text in Hints and ACDI
**Tests:** Layer 2 stray-text check for `<hints>` and `<acdi>` empty elements
**Defect:** Text inside `<acdi>` (should be empty) and `<hints>` (child-only content)
| Check | Expected |
|-------|----------|
| Validate | **Error:** stray text in `<hints>` and `<acdi>` |

---

### `cdi/33_stray_text_deep_nesting.xml` — Stray Text in Deep Nesting (5 Levels)
**Tests:** Stray text at every level in 5-deep group nesting
**Defect:** `STRAY_L1` through `STRAY_L5` in nested groups, `STRAY_LEAF` in deepest `<int>`
| Check | Expected |
|-------|----------|
| Validate | **Error:** at least 6 stray-text errors across all nesting levels |

---

### `cdi/34_stray_text_all_variable_types.xml` — Stray Text in All Variable Types
**Tests:** Layer 2 stray-text check for every CDI 1.4 variable type: `<string>`, `<int>`, `<float>`, `<eventid>`, `<action>`, `<blob>`
**Defect:** Text `STRAY` inside each variable element
| Check | Expected |
|-------|----------|
| Validate | **Error:** at least 6 stray-text errors, one per variable type |

---

### `cdi/35_stray_text_deep_nesting_clean.xml` — Deep Nesting Clean (No False Positives)
**Tests:** Same 5-level nesting structure as test 33 but with no stray text
| Check | Expected |
|-------|----------|
| Validate | No errors |
| Memory Map | Parses, no overlaps |
| Compile | No duplicates |

---

### `cdi/36_bad_attributes_segment.xml` — Bad Segment Attributes
**Tests:** Layer 2 attribute validation for `<segment>` — missing space, non-numeric space, out-of-range space
| Check | Expected |
|-------|----------|
| Validate | **Error:** missing space, invalid space number |

---

### `cdi/37_bad_attributes_int_float.xml` — Bad Int/Float/String Size Attributes
**Tests:** Layer 2 size attribute validation — int sizes 3/5/0, float sizes 3/1, missing string size
| Check | Expected |
|-------|----------|
| Validate | **Error:** invalid int size; **Warning:** unusual float size, missing string size |

---

### `cdi/38_bad_attributes_checkbox_map.xml` — Bad Checkbox Map Relations
**Tests:** Layer 2 checkbox/map relationship — 0, 1, and 3 relations (exactly 2 required)
| Check | Expected |
|-------|----------|
| Validate | **Error:** checkbox map relation count wrong (3 errors) |

---

### `cdi/39_bad_singleton_duplicates.xml` — Duplicate Singleton Elements
**Tests:** Layer 2 singleton check — multiple `<identification>` and `<acdi>` under `<cdi>`
| Check | Expected |
|-------|----------|
| Validate | **Error:** duplicate identification, duplicate acdi |

---

## FDI Test Files

### `fdi/01_stray_text_in_function.xml` — FDI Stray Text in Containers
**Tests:** Stray text inside `<segment>`, `<group>`, and `<function>` elements
| Check | Expected |
|-------|----------|
| Validate | **Error:** stray text in segment, group, and function |

---

### `fdi/02_stray_text_deep_nesting.xml` — FDI Stray Text Deep Nesting
**Tests:** Stray text at multiple levels: segment > group > group > group > function
| Check | Expected |
|-------|----------|
| Validate | **Error:** at least 4 stray-text errors across nesting levels |

---

### `fdi/03_bad_attributes.xml` — FDI Bad Attributes
**Tests:** Missing kind, invalid kind, missing name, missing number, wrong space
| Check | Expected |
|-------|----------|
| Validate | **Error:** missing kind, invalid kind; **Warning:** missing number, wrong space |

---

### `fdi/04_clean_no_false_positives.xml` — FDI Clean (No False Positives)
**Tests:** Valid FDI with nested groups and whitespace — no stray text
| Check | Expected |
|-------|----------|
| Validate | No errors, no warnings |

---

### `fdi/05_malformed_xml.xml` — FDI Malformed XML (Layer 1)
**Tests:** Unclosed `<name>` tag — DOMParser (Layer 1) returns immediately with a parse error.  No Layer 2 or Layer 3 checks run.
| Check | Expected |
|-------|----------|
| Validate | **Error:** exactly 1 XML parse error; validation stops |

---

### `fdi/06_wrong_root.xml` — FDI Wrong Root Element
**Tests:** Root element is `<functions>` instead of `<fdi>`.  Layer 2 root check flags the wrong name.
| Check | Expected |
|-------|----------|
| Validate | **Error:** root element must be `<fdi>` |

---

### `fdi/07_multiple_segments.xml` — FDI Multiple Segments
**Tests:** Two `<segment>` elements in one FDI.  FDI allows only one segment; the singleton check produces an error.
| Check | Expected |
|-------|----------|
| Validate | **Error:** FDI allows only one `<segment>` |

---

### `fdi/08_duplicate_function_numbers.xml` — FDI Duplicate Function Numbers
**Tests:** F0 assigned to three functions; F2 assigned to two functions.  Layer 3 duplicate-number check warns on each repeated number (first use is fine).
| Check | Expected |
|-------|----------|
| Validate | **Warning:** duplicate function number 0 (twice); duplicate function number 2 (once) — at least 3 warnings |

---

### `fdi/09_function_numbers_out_of_range.xml` — FDI Function Numbers Out of Range
**Tests:** Functions F29, F50, F128 — beyond the standard DCC range of 0–28.  Layer 3 produces info messages for each.  F0, F10, F28 are in range and produce no message.
| Check | Expected |
|-------|----------|
| Validate | **Info:** out-of-range messages for F29, F50, F128 — at least 3 info messages |

---

### `fdi/10_no_segment.xml` — FDI No Segment
**Tests:** Completely empty `<fdi>` root with no `<segment>`.  Layer 3 warns that FDI needs at least one segment.
| Check | Expected |
|-------|----------|
| Validate | **Warning:** no `<segment>` element found |

---

### `cdi/40_identification_empty_fields.xml` — Empty Identification Fields
**Tests:** `<identification>` is present but both `<manufacturer>` and `<model>` are empty strings.  Layer 3 info messages flag each empty sub-field.
| Check | Expected |
|-------|----------|
| Validate | **Info:** empty manufacturer; **Info:** empty model — at least 2 info messages |

---

### `cdi/41_space_254_eventids.xml` — Event IDs in Space 254
**Tests:** A segment in space 0xFE (254, "All Memory") that contains `<eventid>` elements.  Layer 3 flags this as unusual.
| Check | Expected |
|-------|----------|
| Validate | **Info:** event IDs found in space 0xFE — at least 1 info message |

---

### `cdi/42_wrong_child_under_cdi.xml` — Wrong Children Under CDI Root
**Tests:** `<group>`, `<string>`, and `<int>` placed directly under `<cdi>` instead of inside a `<segment>`.  Layer 2 produces a warning for each unexpected direct child.
| Check | Expected |
|-------|----------|
| Validate | **Warning:** unexpected child `<group>`, `<string>`, `<int>` of `<cdi>` — at least 3 warnings |

---

### `cdi/43_no_segments.xml` — CDI No Segments
**Tests:** CDI with `<identification>` and `<acdi/>` but no `<segment>` elements at all.  Layer 3 flags the missing space 253 segment.
| Check | Expected |
|-------|----------|
| Validate | **Info:** no segment in space 0xFD (253) — at least 1 info message |

---

### `cdi/44_kitchen_sink_valid.xml` — Kitchen Sink Valid CDI
**Tests:** Exercises every tag and every attribute defined in CDI 1.4 at least once in a structurally correct document.  This is a regression guard — any false positive in the validator will cause it to fail.

Tag coverage: `acdi`, `action`, `blob`, `buttonText`, `cdi`, `checkbox`, `default`, `description`, `dialogText`, `eventid`, `float`, `group`, `hardwareVersion`, `hints`, `identification`, `int`, `link`, `manufacturer`, `map`, `max`, `min`, `model`, `name`, `property`, `radiobutton`, `readOnly`, `relation`, `repname`, `segment`, `slider`, `softwareVersion`, `string`, `value`, `visibility`

Attribute coverage: `fixed`, `formatting`, `hidden`, `hideable`, `immediate`, `mode`, `offset`, `origin`, `ref`, `replication`, `showValue`, `size`, `space`, `tickSpacing`, `var`

| Check | Expected |
|-------|----------|
| Validate | Zero errors, zero warnings |

---

### `fdi/11_kitchen_sink_valid.xml` — Kitchen Sink Valid FDI
**Tests:** Exercises every tag and every attribute defined in FDI 1.1 at least once.  Includes all three function kinds (`binary`, `momentary`, `analog`), nested groups, and `min`/`max` children on analog functions.

Tag coverage: `description`, `fdi`, `function`, `group`, `max`, `min`, `name`, `number`, `segment`

Attribute coverage: `kind` (binary/momentary/analog), `origin`, `size`, `space`

| Check | Expected |
|-------|----------|
| Validate | Zero errors, zero warnings |

---

### `cdi/30_stray_text_whitespace_ok.xml` — extended (gap fill)
The previously-existing test 30 was extended to also include `<link>`, `<visibility>`, `<readOnly>`, `<slider>`, and `<radiobutton>` — all five CDI tags that had zero coverage before.  The extended section uses the same heavy-whitespace style as the rest of the file so it simultaneously tests that whitespace inside these elements is not falsely flagged.

---

## Test Folders

| Folder | Purpose |
|--------|---------|
| `test/cdi/` | CDI XML test cases for the CDI editor and validator |
| `test/fdi/` | FDI XML test cases for the FDI editor and validator |

## How to Run

### Headless (Node.js — fastest, runs all 55 tests at once)
```
node run_tests.js
```
Requires `jsdom` (`npm install jsdom`).  Exits 0 on all-pass, 1 on any failure.
The script lives at the repository root alongside this folder.

### Browser automated (recommended for visual review)
1. Open `test_runner.html` in a browser
2. Select all files in `test/cdi/` and drag them onto the drop zone
3. Results appear immediately — failed tests auto-expand

### Manual
1. Open `node_wizard.html` in a browser
2. Select "Typical" node type
3. Click the CDI tile to enter the CDI editor
4. Use "Open XML" to load each test file
5. Check the validation footer, memory map views, and stats bar
