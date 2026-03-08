# OpenLCB C Library — Doxygen Style Guide

**Version:** 2.0
**Date:** 08 Mar 2026
**Purpose:** Complete rules for writing Doxygen comments — how much to write, what tags to use, and what to avoid

**Changes in v2.0:**
- Combined Doxygen_StyleGuide.md (v1.2) and C_Documentation_Guide_v1_7.md (v2.1) into a single document
- All 10 critical rules, block sizing, complete examples, and checklists in one place

---

## The Problem This Guide Solves

The goal of documentation is to help, not to hide the code.  A huge block of Doxygen
comments above a two-line function makes the file harder to read, not easier.  When
you open a `.c` or `.h` file you should be able to scan down it and find the functions
quickly.  Comments that go on for 40 lines before a 3-line function work against that.

**The rule of thumb:** documentation length should be roughly proportional to function
complexity.  A one-liner getter needs a one-liner comment.  A complex state-machine
function earns a full block.

---

## Comment Block Sizes

There are three sizes of Doxygen block.  Pick the smallest one that is honest.

### Size 1 — Inline (trivial functions)

Use for simple getters, setters, and one-line helpers where the function name and
signature already explain everything.

```c
        /** @brief Returns the number of BASIC messages currently allocated. */
    extern uint16_t OpenLcbBufferStore_basic_messages_allocated(void);
```

No `@details`, no `@note`, no `@see`.  If the function name says it all, let it.

---

### Size 2 — Short block (most functions)

Use for functions that need a sentence or two of context, important warnings, and
parameter/return descriptions.  This covers the majority of the codebase.

```c
        /**
         * @brief Increments the reference count on an allocated buffer.
         *
         * @details A buffer may be held by more than one queue at the same time.
         * Each holder increments the count.  The buffer is not freed until the
         * count reaches zero.
         *
         * @param msg  Pointer to the message buffer.
         *
         * @warning NULL pointer causes undefined behavior — caller must ensure validity.
         *
         * @see OpenLcbBufferStore_free_buffer
         */
    extern void OpenLcbBufferStore_inc_reference_count(openlcb_msg_t *msg);
```

Keep `@details` to 2–4 sentences.  Use `@warning` only when missing it causes a crash
or silent data corruption.  Skip tags that add no new information.

---

### Size 3 — Full block (complex functions only)

Use for functions with non-obvious behavior, multiple failure modes, or an algorithm
worth spelling out in the `.c` file.  This is for the minority of functions.

```c
        /**
         * @brief Allocates a new buffer of the specified payload type.
         *
         * @details Searches the appropriate pool segment for an unallocated buffer and
         * returns it ready to use with the reference count set to 1.  Returns NULL when
         * the pool segment is exhausted — the caller is always responsible for checking.
         *
         * @param payload_type  Type of buffer requested: BASIC, DATAGRAM, SNIP, or STREAM.
         *
         * @return Pointer to the allocated buffer, or NULL if the pool is exhausted.
         *
         * @warning NULL return means the pool is full — caller MUST check before use.
         * @warning NOT thread-safe.
         *
         * @see OpenLcbBufferStore_free_buffer
         */
    extern openlcb_msg_t *OpenLcbBufferStore_allocate_buffer(payload_type_enum payload_type);
```

The `.c` file version of a full-block function adds an `Algorithm:` section (see
Rule #3 for details).

---

## What to Leave Out

These things make blocks longer without making them more useful.

**Do not write obvious things.**  If the function is named `get_count` and returns
`uint16_t`, do not write `@return The count as a uint16_t`.  Write what the count
*means*, or leave out `@return` entirely if the name is clear enough.

**Do not repeat the function name in `@brief`.**
- Bad:  `@brief OpenLcbBufferStore_initialize initializes the buffer store`
- Good: `@brief Initializes the buffer store`

**Do not write `@param None` or `@return None`.**  If there are no parameters,
omit the `@param` tag completely.  Same for `void` returns.

**Do not add `@note` or `@see` just to fill space.**  Every tag should add
information the reader could not get from the function signature and the `@brief`
line alone.

**Do not use `@remark` to restate the search strategy.**  That is obvious from
reading the code and adds nothing.

**Do not use Big-O notation.**  Write in plain language (see Rule #2).

---

## Quick Reference — Choosing a Block Size

| Situation | Block size |
|---|---|
| Simple getter / setter / flag set / flag clear | Inline (Size 1) |
| Most public functions | Short block (Size 2) |
| Complex algorithm or multiple failure modes | Full block (Size 3) |
| Static helper with obvious purpose | Inline (Size 1) |
| Static helper with subtle behavior | Short block (Size 2) |

When in doubt, go smaller.  You can always add more later.

---

## Application-Facing Files — Higher Documentation Standard

Any `.h` or `.c` file in `/src/openlcb/` that contains "application" in its filename
is a user-facing API file and requires one level higher of Doxygen documentation
coverage than the table above would otherwise suggest.

In practice this means:

| Normal block size | Minimum for application files |
|---|---|
| Inline (Size 1) | Short block (Size 2) |
| Short block (Size 2) | Full block (Size 3) |
| Full block (Size 3) | Full block (Size 3) |

These files are the primary interface between the library and application code.
Complete documentation here directly helps end users understand how to configure
and interact with the library.

---

## Critical Documentation Rules

### RULE #1: NEVER USE @param FOR FUNCTIONS WITHOUT PARAMETERS

**WRONG:**
```c
/**
 * @brief Initializes the buffer store
 * @param None          <- WRONG! Remove this line!
 * @return None         <- WRONG! Remove this line!
 */
void BufferStore_initialize(void);
```

**CORRECT:**
```c
    /**
     * @brief Initializes the buffer store
     *
     * @details Sets up the pre-allocated message pool...
     *
     * @warning MUST be called during initialization
     *
     * @see BufferStore_allocate - Uses initialized pool
     */
void BufferStore_initialize(void);
```

Quick rules for `@param` and `@return`:
1. Function has NO parameters → DO NOT include any `@param` tags
2. Function returns `void` → DO NOT include `@return` tag
3. Function has parameters → Include `@param` for EACH parameter
4. Function returns non-void → Include `@return` tag with description

---

### RULE #2: NEVER INCLUDE BIG-O NOTATION

This is embedded C code, not academic algorithm documentation.  Describe performance
in plain language.

**WRONG:**
```c
/**
 * @note Time complexity: O(n)          <- WRONG!
 * @remark O(n) worst case              <- WRONG!
 */
```

**CORRECT:**
```c
/**
 * @note Linear search through all entries
 */
```

Plain language equivalents:
- "Linear search through all entries" instead of "O(n)"
- "Constant time lookup" instead of "O(1)"
- "Searches entire list" instead of "O(n) complexity"

---

### RULE #3: NEVER INCLUDE ALGORITHM DETAILS IN HEADER FILES

Algorithm steps (`Algorithm:`, numbered steps, implementation flow) belong in `.c`
files only.  Header files describe WHAT and WHEN.  Implementation files describe HOW.

**WRONG — in a .h file:**
```c
/**
 * @details Algorithm:                   <- WRONG! No "Algorithm:" in headers!
 * -# Search buffer pool for free slot   <- WRONG! No implementation steps!
 * -# Mark buffer as allocated           <- WRONG!
 */
```

**CORRECT — .h file:**
```c
        /**
         * @brief Allocates a new buffer from the pool
         *
         * @details Searches the buffer pool for an available buffer and returns
         * a pointer to it. The buffer is marked as allocated and ready for use.
         *
         * @return Pointer to allocated buffer, or NULL if pool is exhausted
         *
         * @warning Returns NULL when pool exhausted - caller MUST check
         */
    extern openlcb_msg_t* BufferStore_allocate(void);
```

**CORRECT — .c file:**
```c
    /**
     * @brief Allocates a new buffer from the pool
     *
     * @details Algorithm:
     * -# Iterate through buffer pool array
     * -# Check each buffer's allocated flag
     * -# When unallocated buffer found:
     *    - Set allocated flag to true
     *    - Set reference count to 1
     *    - Clear message structure
     *    - Return pointer to buffer
     * -# If no free buffers, return NULL
     *
     * @return Pointer to allocated buffer, or NULL if pool is exhausted
     *
     * @warning Returns NULL when pool exhausted - caller MUST check
     */
    openlcb_msg_t* BufferStore_allocate(void) {
        // Implementation...
    }
```

**Header files (.h) should have:**
- WHAT the function does (high-level purpose)
- WHEN to use it (use cases)
- WHAT it returns and error conditions
- Important warnings and requirements
- Cross-references to related functions

**Header files (.h) should NOT have:**
- HOW it works (algorithm steps)
- Implementation details
- Step-by-step processing flow
- Internal state changes
- Data structure manipulation details

**Implementation files (.c) should have:**
- Everything from the header PLUS
- `Algorithm:` section with step-by-step details
- Internal state changes
- Data structure manipulation
- Processing flow details

---

### RULE #4: DOXYGEN BLOCK IS ALWAYS AT CODE COLUMN + 4

The Doxygen block must always be indented 4 spaces MORE than the code it documents.
The rule is purely relative — find the column where the code starts, add 4, and put
the `/**` there.  File type (.c or .h) does not change this.

Code at column 0 → block at column 4:
```c
    /**
     * @brief Does something useful.
     */
void SomeModule_do_something(void) {
    // body at column 4, not affected
}
```

Code at column 4 → block at column 8:
```c
        /**
         * @brief Does something useful.
         */
    extern void SomeModule_do_something(void);
```

Code at column 8 → block at column 12:
```c
            /**
             * @brief Does something useful.
             */
        extern void SomeModule_do_something(void);
```

**Quick lookup:**

| Code starts at column | Doxygen block starts at column |
|---|---|
| 0  | 4  |
| 4  | 8  |
| 8  | 12 |
| 12 | 16 |

---

### RULE #5: NEVER START @warning/@note/@retval/@attention/@remark WITH A PARAMETER NAME

Doxygen parsers scan these sections for parameter names.  When a parameter name
appears at the start, Doxygen treats it as a duplicate `@param` and emits warnings.

**Tags affected:** `@warning`, `@note`, `@attention`, `@retval`, `@exception`,
`@throw`, `@throws`, `@remark`

**WRONG:**
```c
 * @warning Returns NULL if alias is invalid     <- "alias" triggers duplicate @param
 * @note The alias parameter must be non-zero    <- "alias" triggers duplicate @param
 * @retval NULL if buffer is invalid             <- "buffer" may trigger duplicate @param
```

**CORRECT:**
```c
 * @warning Invalid alias values will cause NULL return
 * @note Zero values are not permitted
 * @retval NULL Invalid buffer or allocation failure
```

**Safe rephrasing patterns:**
- "Returns NULL if alias is invalid" → "Invalid alias values will cause NULL return"
- "alias must be non-zero" → "Zero values are not permitted"
- "msg parameter cannot be NULL" → "NULL pointers are not permitted"

---

### RULE #6: ALWAYS USE @verbatim FOR @param IN .c FILES

When function documentation exists in BOTH `.h` and `.c` files, Doxygen will detect
duplicate `@param` sections.  Wrap ALL `@param` tags in the `.c` file with
`@verbatim`/`@endverbatim`.

**`.h` file** — canonical `@param` tags (rendered in the HTML output):
```c
        /**
         * @brief Registers the dependency-injection interface for this module.
         *
         * @param interface_handler Pointer to a populated
         *        @ref interface_handler_t. Must remain valid for the
         *        lifetime of the application.
         *
         * @warning Must be called before any other module function.
         */
    extern void SomeModule_initialize(const interface_handler_t *interface_handler);
```

**`.c` file** — `@param` wrapped so Doxygen ignores it:
```c
    /**
     * @brief Registers the dependency-injection interface for this module.
     *
     * @verbatim
     * @param interface_handler Pointer to a populated interface_handler_t.
     * @endverbatim
     *
     * @warning Must be called before any other module function.
     */
void SomeModule_initialize(const interface_handler_t *interface_handler) {

    _interface = interface_handler;

}
```

**Key points:**
- The `.h` file is the single source of truth for `@param` documentation.
- The `.c` file repeats the parameters inside `@verbatim` only so a reader of the
  `.c` file can see them without opening the header.
- `@ref` links do not render inside `@verbatim`, so use plain type names there.
- Tags **outside** the `@verbatim` block (`@brief`, `@details`, `@warning`,
  `@return`, `@see`) are processed normally.
- Only wrap `@param` when the `.h` file already has `@param` tags for the same
  function.  If the `.h` file does **not** document `@param` (or there is no `.h`
  declaration at all, e.g. static/private functions), write `@param` tags normally
  in the `.c` file **without** `@verbatim` wrapping.

**Placement order in `.c` file blocks:**
```c
/**
 * @brief Function description
 *
 * @details Algorithm details here...
 *
 * @verbatim
 * @param x First parameter        <- Inside verbatim
 * @param y Second parameter       <- Inside verbatim
 * @endverbatim
 *
 * @return Description              <- Outside verbatim (Doxygen processes this)
 *
 * @warning Important notes         <- Outside verbatim (Doxygen processes this)
 */
```

---

### RULE #7: NEVER CHANGE THE NAMES OF ANY CODE IDENTIFIERS WITHOUT ASKING

In ALL files (.c AND .h), do not change constants, function names, variables, etc.
without a discussion first.

---

### RULE #8: ALWAYS UPDATE @date WHEN MODIFYING A FILE

Every time a `.c` or `.h` file is modified, the `@date` field in the file header
MUST be updated to the current date.  The `@date` records when the file was last
changed, not when it was originally created.

**WRONG — stale date after modification:**
```c
/**
 * @file alias_mappings.h
 * @brief Fixed-size buffer mapping CAN aliases to Node IDs.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026        <- WRONG if file was modified later
 */
```

**CORRECT:**
```c
/**
 * @file alias_mappings.h
 * @brief Fixed-size buffer mapping CAN aliases to Node IDs.
 *
 * @author Jim Kueneman
 * @date 08 Mar 2026        <- Updated to the date the file was changed
 */
```

**Rules:**
- Update `@date` in BOTH the `.h` and `.c` file whenever either is modified
- Use the format: DD Mon YYYY (e.g. 08 Mar 2026)
- The `@date` appears ONLY in the file header — not in individual function blocks
- When running a documentation update pass over multiple files, each file gets today's date

---

### RULE #9: USE @ref FOR CUSTOM TYPES IN @param AND @return

When a `@param` or `@return` involves a custom type (any `typedef`, `struct`, or
`enum` defined in the project — not a standard C type like `uint16_t`), use
`@ref` to create a clickable link in the generated HTML.

```c
        /**
         * @brief Registers a CAN alias / Node ID pair in the buffer.
         *
         * @param alias    12-bit CAN alias (valid range: 0x001–0xFFF).
         * @param node_id  48-bit OpenLCB @ref node_id_t.
         *
         * @return Pointer to the registered @ref alias_mapping_t entry, or NULL on failure.
         */
    extern alias_mapping_t *AliasMappings_register(uint16_t alias, node_id_t node_id);
```

**Rules:**
- Use `@ref TypeName` inline within the `@param` or `@return` text.
- Apply to any project-defined type: `_t` typedefs, `_enum` enumerations, struct names.
- Do NOT use `@ref` for standard C types (`uint8_t`, `uint16_t`, `bool`, `int`, etc.).
- Do NOT use `@ref` for types from standard headers (`size_t`, `FILE`, etc.).
- In `@see` tags, prefer `@ref` for types and plain function names for functions.
- `@ref` links do not render inside `@verbatim`, so use plain type names in `.c` file
  `@param` blocks.

---

### RULE #10: HEADER EXTERN DECLARATIONS MUST INCLUDE @param WITH TYPE LINKS

All `extern` function declarations in header files MUST include `@param` tags for
every parameter.  Any parameter whose type is defined by the library MUST use `@ref`.

**This rule does NOT apply to interface struct function-pointer members.**  Callback
pointers inside `typedef struct { ... } interface_xxx_t;` structs use Size 1 inline
doc (a single `/** @brief ... */` line).  They do not need `@param` tags.

**CORRECT — extern function in .h with @param and @ref links:**
```c
        /**
         * @brief Handle incoming Get Address Space Info command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_get_address_space_info(
            openlcb_statemachine_info_t *statemachine_info);
```

**CORRECT — interface struct member (NO @param needed):**
```c
        /** @brief Optional — Handle Lock/Reserve command. */
    void (*operations_request_reserve_lock)(
            openlcb_statemachine_info_t *statemachine_info,
            config_mem_operations_request_info_t *config_mem_operations_request_info);
```

**WRONG — extern declaration missing @param:**
```c
        /** @brief Handle incoming Get Address Space Info command. */
    extern void ProtocolConfigMemOperationsHandler_get_address_space_info(
            openlcb_statemachine_info_t *statemachine_info);   // <- WRONG: no @param!
```

**Summary:**

| Location | @param required? | @ref for library types? |
|---|---|---|
| `extern` function in `.h` | YES | YES |
| `extern` variable in `.h` | N/A (no params) | YES in @brief if type is library-defined |
| Interface struct callback member | NO (Size 1 brief only) | NO |
| Function definition in `.c` | YES (wrapped in @verbatim) | NO (plain names in verbatim) |

---

## File Header Blocks

Every `.c` and `.h` file gets a file header after the copyright block.  Keep it
short — a paragraph, not a page.  The `@author` and `@date` appear once only.

```c
/**
 * @file openlcb_buffer_store.h
 * @brief Pre-allocated message pool for OpenLCB buffer management.
 *
 * @details Provides fixed-size pools for BASIC, DATAGRAM, SNIP, and STREAM
 * message types.  All memory is allocated at startup; there is no dynamic
 * allocation at runtime.  Must be initialized before any other OpenLCB module.
 *
 * @author Jim Kueneman
 * @date 08 Mar 2026
 */
```

Three to five sentences is enough.  Do NOT repeat `@author` and `@date` in the
function blocks — they belong in the file header only.

**Always update `@date` to the current date whenever a file is modified** (Rule #8).

---

## Static Variable Comments

Static module-level variables use the brief single-line form:

```c
/** @brief Current number of allocated BASIC messages. */
static uint16_t _buffer_store_basic_messages_allocated = 0;

/** @brief Peak number of BASIC messages allocated simultaneously. */
static uint16_t _buffer_store_basic_max_messages_allocated = 0;
```

Do not give variables a multi-line block unless they have genuinely complex semantics.

---

## Complete Examples

### Example 1: Function with NO parameters, returns void

**Header File (.h):**
```c
        /**
         * @brief Initializes the OpenLcb Buffer Store
         *
         * @details Sets up the pre-allocated message pool for use by the system.
         * Must be called once during startup before any buffer operations.
         *
         * @warning MUST be called exactly once during initialization
         * @warning NOT thread-safe
         *
         * @attention Call before OpenLcbBufferFifo_initialize()
         *
         * @see OpenLcbBufferStore_allocate_buffer - Allocates from initialized pool
         */
    extern void OpenLcbBufferStore_initialize(void);
```

**Implementation File (.c):**
```c
    /**
     * @brief Initializes the OpenLcb Buffer Store
     *
     * @details Algorithm:
     * -# Clear all message structures to zero
     * -# Link each message to its appropriate payload buffer
     * -# Organize buffers by type: [BASIC][DATAGRAM][SNIP][STREAM]
     * -# Reset all allocation counters and telemetry
     *
     * @warning MUST be called exactly once during initialization
     * @warning NOT thread-safe
     *
     * @attention Call before OpenLcbBufferFifo_initialize()
     *
     * @see OpenLcbBufferStore_allocate_buffer - Allocates from initialized pool
     */
void OpenLcbBufferStore_initialize(void) {
    // Implementation...
}
```

### Example 2: Function with NO parameters, returns value

**Header File (.h):**
```c
        /**
         * @brief Returns the number of BASIC messages currently allocated
         *
         * @details Provides real-time count of allocated BASIC-type message buffers.
         * Useful for monitoring system load and detecting buffer leaks.
         *
         * @return Number of BASIC sized messages currently allocated (0 to USER_DEFINED_BASIC_BUFFER_DEPTH)
         *
         * @note This is a live count that changes as buffers are allocated and freed
         *
         * @see OpenLcbBufferStore_basic_messages_max_allocated - Peak usage counter
         */
    extern uint16_t OpenLcbBufferStore_basic_messages_allocated(void);
```

**Implementation File (.c):**
```c
    /**
     * @brief Returns the number of BASIC messages currently allocated
     *
     * @details Algorithm:
     * -# Access static allocation counter for BASIC buffers
     * -# Return current value
     *
     * @return Number of BASIC sized messages currently allocated (0 to USER_DEFINED_BASIC_BUFFER_DEPTH)
     *
     * @note This is a live count that changes as buffers are allocated and freed
     *
     * @see OpenLcbBufferStore_basic_messages_max_allocated - Peak usage counter
     */
uint16_t OpenLcbBufferStore_basic_messages_allocated(void) {
    return _basic_messages_allocated;
}
```

### Example 3: Function with parameters, returns void

**Header File (.h):**
```c
        /**
         * @brief Increments the reference count on an allocated buffer
         *
         * @details Increases the buffer's reference count to indicate that an additional
         * part of the system is now holding a pointer to this buffer.
         *
         * @param msg Pointer to @ref openlcb_msg_t buffer to increment reference count
         *
         * @warning NULL pointers will cause undefined behavior - no NULL check performed
         * @warning NOT thread-safe
         *
         * @attention Always pair with corresponding free_buffer() call
         *
         * @see OpenLcbBufferStore_free_buffer - Decrements reference count
         */
    extern void OpenLcbBufferStore_inc_reference_count(openlcb_msg_t *msg);
```

**Implementation File (.c):**
```c
    /**
     * @brief Increments the reference count on an allocated buffer
     *
     * @details Algorithm:
     * -# Access msg->state.reference_count field
     * -# Increment value by 1
     * -# No bounds checking performed
     *
     * @verbatim
     * @param msg Pointer to openlcb_msg_t buffer to increment reference count
     * @endverbatim
     *
     * @warning NULL pointers will cause undefined behavior - no NULL check performed
     * @warning NOT thread-safe
     *
     * @attention Always pair with corresponding free_buffer() call
     *
     * @see OpenLcbBufferStore_free_buffer - Decrements reference count
     */
void OpenLcbBufferStore_inc_reference_count(openlcb_msg_t *msg) {
    msg->state.reference_count++;
}
```

### Example 4: Function with parameters, returns value

**Header File (.h):**
```c
        /**
         * @brief Allocates a new buffer of the specified payload type
         *
         * @details Searches the appropriate buffer pool segment for an unallocated buffer.
         * The buffer is returned ready for use with reference count set to 1.
         *
         * @param payload_type Type of buffer requested: BASIC, DATAGRAM, SNIP, or STREAM (@ref payload_type_enum)
         *
         * @return Pointer to allocated @ref openlcb_msg_t buffer, or NULL if pool exhausted or invalid type
         *
         * @warning Returns NULL when buffer pool is exhausted - caller MUST check for NULL
         * @warning NOT thread-safe
         *
         * @see OpenLcbBufferStore_free_buffer - Frees allocated buffer
         */
    extern openlcb_msg_t* OpenLcbBufferStore_allocate_buffer(payload_type_enum payload_type);
```

**Implementation File (.c):**
```c
    /**
     * @brief Allocates a new buffer of the specified payload type
     *
     * @details Algorithm:
     * -# Determine buffer pool segment based on payload_type
     * -# Iterate through segment to find unallocated buffer
     * -# When found:
     *    - Mark buffer as allocated
     *    - Clear message structure
     *    - Set reference count to 1
     *    - Update allocation counter
     *    - Return pointer to buffer
     * -# If no free buffer found, return NULL
     *
     * @verbatim
     * @param payload_type Type of buffer requested (BASIC, DATAGRAM, SNIP, or STREAM)
     * @endverbatim
     *
     * @return Pointer to allocated openlcb_msg_t buffer, or NULL if pool exhausted or invalid type
     *
     * @warning Buffer pool exhaustion returns NULL - caller MUST check for NULL
     * @warning NOT thread-safe
     *
     * @see OpenLcbBufferStore_free_buffer - Frees allocated buffer
     */
openlcb_msg_t* OpenLcbBufferStore_allocate_buffer(payload_type_enum payload_type) {
    // Implementation...
}
```

---

## Summary Checklist

**Header Files (.h) Documentation Must Have:**
- [ ] Doxygen block at declaration column + 4
- [ ] `@brief` with one-line summary
- [ ] `@details` with WHAT and WHEN (NO HOW/Algorithm)
- [ ] `@param` for every parameter with `@ref` for library types (Rule #10)
- [ ] `@return` only if function returns non-void
- [ ] `@warning` for critical issues (NEVER start with parameter names — Rule #5)
- [ ] NO `Algorithm:` section
- [ ] NO implementation steps
- [ ] NO Big-O notation

**Implementation Files (.c) Documentation Must Have:**
- [ ] Doxygen block at function column + 4
- [ ] `@brief` with one-line summary
- [ ] `@details` with `Algorithm:` and step-by-step HOW
- [ ] `@param` wrapped in `@verbatim`/`@endverbatim` (Rule #6)
- [ ] `@return` only if function returns non-void
- [ ] `@warning` for critical issues (NEVER start with parameter names — Rule #5)
- [ ] NO bare `@param` tags (always wrap in `@verbatim` when `.h` file has `@param`)
- [ ] NO Big-O notation

---

## Before You Commit — Mental Checklist

- Can I find the function signature by scanning down the file quickly?  If not, the
  comment block is probably too large.
- Does every tag in the block add something the function name and signature do not
  already say?
- Did I pick the smallest block size that is still honest?
- Is the block at **code column + 4**?  (Count the spaces on the code line, add 4,
  check the `/**` line.)
- Am I using `@param None` or `@return None` anywhere?  (Remove them.)
- Does any `@warning`, `@note`, or `@retval` text start with a parameter name?
- Is `@author` / `@date` only in the file header, not repeated in every function?
- Is the `@date` in the file header updated to today's date?
- Are any `@remark` tags just restating the obvious?  (Remove them.)
- Does every custom type in a `@param` or `@return` have a `@ref` link?
- Does every `extern` function declaration in a `.h` file have `@param` tags?
  (Interface struct callback members are exempt.)
