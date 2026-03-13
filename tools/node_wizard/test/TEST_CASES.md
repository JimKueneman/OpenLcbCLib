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

## Test Folders

| Folder | Purpose |
|--------|---------|
| `test/cdi/` | CDI XML test cases for the CDI editor and validator |
| `test/fdi/` | FDI XML test cases (placeholder — FDI editor not yet implemented) |

## How to Run

### Automated (recommended)
1. Open `test_runner.html` in a browser
2. Select all files in `test/cdi/` and drag them onto the drop zone
3. Results appear immediately — failed tests auto-expand

### Manual
1. Open `node_wizard.html` in a browser
2. Select "Typical" node type
3. Click the CDI tile to enter the CDI editor
4. Use "Open XML" to load each test file
5. Check the validation footer, memory map views, and stats bar
