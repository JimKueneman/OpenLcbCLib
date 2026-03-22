# XmlToArray

Converts an XML file (CDI, FDI, or any XML) into a C-style hex byte array
for embedding in OpenLCB node parameters. XML comments are always stripped.
By default whitespace (indentation, newlines, blank lines) is also stripped
to minimize the byte array size. A null terminator (0x00) is always appended.

## Usage

    python XmlToArray.py [options] [input.xml]

## Options

    -h, --help           Show help message
    -f, --format         Pretty-print the XML before converting
    -w, --whitespace     Preserve whitespace (indentation, newlines)
    -n, --name NAME      C variable name for the array (default: _cdi_data)
    -t, --type TYPE      Shorthand: 'cdi' or 'fdi' (sets variable name)
    -o, --output PATH    Output base path (generates PATH.c and PATH.h)
    --header             Also generate a .h file with extern declaration
    --author NAME        Author name for the file header comment
    --company NAME       Company/copyright holder for the file header
    --license PATH       Path to custom license text file for the header

If no input file is given, defaults to "sample.xml".

## Modes

**Default mode** (no --header):
Outputs `<input>.c` containing a `static const uint8_t` array fragment
suitable for pasting into an existing `openlcb_user_config.c` file.

**Standalone mode** (--header):
Generates a `.c`/`.h` pair with BSD 2-clause license header (or custom
license via --license), extern linkage, C++ guards, and doxygen tags.
The `.c` file `#include`s the `.h` and the array can be referenced from
`openlcb_user_config.c` via `#include`.

## Quick Examples

    # Fragment for copy/paste
    python XmlToArray.py my_cdi.xml
        -> my_cdi.xml.c  (static const uint8_t _cdi_data[])

    # Standalone .c/.h pair
    python XmlToArray.py -t cdi --header -o my_cdi my_cdi.xml
        -> my_cdi.c + my_cdi.h  (extern const uint8_t _cdi_data[])

    # FDI for a train node
    python XmlToArray.py -t fdi --header -o train_fdi train_fdi.xml
        -> train_fdi.c + train_fdi.h  (extern const uint8_t _fdi_data[])

    # With author, company, and custom license
    python XmlToArray.py --author "Jane Smith" --company "Acme Rail" \
        --license my_license.txt -t cdi --header -o my_cdi my_cdi.xml
        -> my_cdi.c + my_cdi.h with custom license header

---

## Full Walkthrough: Separating CDI/FDI from node_parameters

This shows how to move CDI and FDI hex arrays out of `openlcb_user_config.c`
into their own generated files, so that updating the XML and recompiling is
all you need after a CDI change.

### Starting point

A typical project has everything in one file:

    my_project/
      openlcb_user_config.c    # CDI/FDI hex arrays inline in the struct
      openlcb_user_config.h
      my_cdi.xml               # the source XML you originally converted by hand
      my_fdi.xml               # source FDI XML (train nodes only)

And somewhere else on disk you may have a license file:

    /Users/you/licenses/my_license.txt

### Step 1 — Generate the standalone files

Run from the repo root (adjust paths to match your layout):

    # CDI
    python3 tools/xml_to_array/XmlToArray.py \
        -t cdi --header \
        -o my_project/cdi_data \
        --author "Jim Kueneman" \
        --company "Mustangpeak Engineering" \
        --license /Users/you/licenses/my_license.txt \
        my_project/my_cdi.xml

This produces two files:

    my_project/cdi_data.h   — extern declaration
    my_project/cdi_data.c   — the hex byte array

For train nodes that also have FDI:

    # FDI
    python3 tools/xml_to_array/XmlToArray.py \
        -t fdi --header \
        -o my_project/fdi_data \
        --author "Jim Kueneman" \
        --company "Mustangpeak Engineering" \
        --license /Users/you/licenses/my_license.txt \
        my_project/my_fdi.xml

### Step 2 — Update openlcb_user_config.c

Delete the old inline hex array from the struct and replace it with an
`#include` and a pointer.

Before:

```c
#include "openlcb_user_config.h"
#include "openlcb_c_lib/openlcb/openlcb_types.h"
#include "openlcb_c_lib/openlcb/openlcb_defines.h"

const node_parameters_t OpenLcbUserConfig_node_parameters = {

    // ... snip, protocol_support, address spaces ...

    .address_space_configuration_definition.highest_address = 971,

    // ... hundreds of lines of hex ...
    .cdi = {
        0x3C, 0x3F, 0x78, 0x6D, ...
        ...
        0x00
    },

    .fdi = { 0x00 },
};
```

After:

```c
#include "openlcb_user_config.h"
#include "openlcb_c_lib/openlcb/openlcb_types.h"
#include "openlcb_c_lib/openlcb/openlcb_defines.h"
#include "cdi_data.h"                                       // ADD
#include "fdi_data.h"                                       // ADD (train only)

const node_parameters_t OpenLcbUserConfig_node_parameters = {

    // ... snip, protocol_support, address spaces ...

    // Space 0xFF — highest_address auto-computed from the array
    .address_space_configuration_definition.highest_address = sizeof(_cdi_data) - 1,

    // Space 0xFA — train nodes only
    .address_space_train_function_definition_info.highest_address = sizeof(_fdi_data) - 1,

    .cdi = _cdi_data,                                       // pointer to generated array
    .fdi = _fdi_data,                                       // pointer (or NULL if no FDI)
};
```

Key changes:
- `#include "cdi_data.h"` (and `"fdi_data.h"` for train nodes)
- `highest_address` uses `sizeof(_cdi_data) - 1` instead of a hardcoded number
- `.cdi = _cdi_data` instead of an inline hex block
- `.fdi = _fdi_data` (or `.fdi = NULL` if the node has no FDI)

### Step 3 — Add to your build

Add `cdi_data.c` (and `fdi_data.c`) to your build system — CMakeLists.txt,
Makefile, IDE project file, etc. — and recompile.

### Result

    my_project/
      openlcb_user_config.c    # small, readable — just the struct + pointers
      openlcb_user_config.h
      cdi_data.c               # generated — hex array with license header
      cdi_data.h               # generated — extern declaration
      fdi_data.c               # generated — hex array (train only)
      fdi_data.h               # generated — extern declaration (train only)
      my_cdi.xml               # source of truth
      my_fdi.xml               # source of truth (train only)

### Re-generating after XML changes

Edit your source XML, re-run the same command, recompile. The `sizeof()` in
`highest_address` updates automatically — no manual byte counting required.

    python3 tools/xml_to_array/XmlToArray.py \
        -t cdi --header \
        -o my_project/cdi_data \
        --author "Jim Kueneman" \
        --company "Mustangpeak Engineering" \
        --license /Users/you/licenses/my_license.txt \
        my_project/my_cdi.xml

---

## Multi-Node Example: Multiple CDI/FDI Arrays

Applications that define more than one `node_parameters_t` (e.g. a
command station with several virtual node types) need separate CDI and
FDI arrays with distinct names. Use the `-n, --name` switch to give
each array a unique C identifier.

### Step 1 — Generate arrays with unique names

    # Basic node CDI
    python3 tools/xml_to_array/XmlToArray.py \
        -n _cdi_node_a --header \
        -o my_project/cdi_node_a \
        --author "Jim Kueneman" \
        --company "Mustangpeak Engineering" \
        my_project/node_a_cdi.xml

    # Train node CDI (different configuration)
    python3 tools/xml_to_array/XmlToArray.py \
        -n _cdi_node_b --header \
        -o my_project/cdi_node_b \
        --author "Jim Kueneman" \
        --company "Mustangpeak Engineering" \
        my_project/node_b_cdi.xml

    # Train node FDI
    python3 tools/xml_to_array/XmlToArray.py \
        -n _fdi_train --header \
        -o my_project/fdi_train \
        --author "Jim Kueneman" \
        --company "Mustangpeak Engineering" \
        my_project/train_fdi.xml

This produces six files:

    my_project/
      cdi_node_a.c / cdi_node_a.h   — const uint8_t _cdi_node_a[]
      cdi_node_b.c / cdi_node_b.h   — const uint8_t _cdi_node_b[]
      fdi_train.c  / fdi_train.h    — const uint8_t _fdi_train[]

### Step 2 — Wire them into node_parameters structs

```c
#include "openlcb_user_config.h"
#include "openlcb_c_lib/openlcb/openlcb_types.h"
#include "openlcb_c_lib/openlcb/openlcb_defines.h"
#include "cdi_node_a.h"
#include "cdi_node_b.h"
#include "fdi_train.h"

// Basic node — CDI only, no FDI
const node_parameters_t node_a_params = {

    .snip.name = "Basic Node",
    // ... protocol_support, address spaces, etc. ...

    .address_space_configuration_definition.highest_address = sizeof(_cdi_node_a) - 1,

    .cdi = _cdi_node_a,
    .fdi = NULL,
};

// Train node — separate CDI + FDI
const node_parameters_t node_b_params = {

    .snip.name = "Train Node",
    // ... protocol_support, address spaces, etc. ...

    .address_space_configuration_definition.highest_address = sizeof(_cdi_node_b) - 1,
    .address_space_train_function_definition_info.highest_address = sizeof(_fdi_train) - 1,

    .cdi = _cdi_node_b,
    .fdi = _fdi_train,
};
```

### Step 3 — Add all .c files to your build

Add `cdi_node_a.c`, `cdi_node_b.c`, and `fdi_train.c` to your build
system, recompile, and each node type picks up its own CDI/FDI
automatically.

Each array name is independent, so you can mix and match — two node
types could share the same CDI but have different FDI, or vice versa,
simply by pointing `.cdi` or `.fdi` at the same generated array.
