# OpenLCB C Library - Complete Doxygen Documentation Guide

**Version:** 2.1  
**Date:** 28 Feb 2026  
**Author:** Documentation Standards for Jim Kueneman's OpenLCB Implementation  
**Changes in v2.1:**
- Added RULE #10: Header extern declarations MUST include @param with @ref links to library types; interface struct callback members are exempt
**Changes in v2.0:**
- Clarified RULE #4: indentation is always relative (code column + 4), not tied to fixed column numbers or file type
**Changes in v1.9:**
- Added RULE #9: Use @ref to link custom types in @param and @return descriptions
**Changes in v1.8:**
- Added CRITICAL RULE #8: ALWAYS update the @date in the file header to the current date when modifying a file
**Changes in v1.7:** 
- Added CRITICAL RULE #6: ALWAYS wrap @param tags in @verbatim/@endverbatim in .c files when documentation exists in both .h and .c files
- Added CRITICAL RULE #7: NEVER change the names of any code identifiers without asking
**Changes in v1.6:** 
- Added CRITICAL RULE #5: NEVER mention parameter names (anything assigned with a @param tag) within any @warning, @note, @retval, @exception, @throw, @attention, or @remark sections to avoid duplicate @param detection
**Changes in v1.5:** 
- Added CRITICAL RULE #4: ALWAYS indent Doxygen blocks 4 spaces MORE than the code they document. The rule is relative: if code starts at column N, the block starts at column N+4.
**Changes in v1.4:** 
- Added CRITICAL RULE #3: NEVER include algorithm details in header files
**Changes in v1.3:** 
- Added CRITICAL RULE #1: NEVER use @param for functions with no parameters
- Added CRITICAL RULE #2: NEVER include Big-O notation or complexity analysis

---

## Overview
This guide provides comprehensive Doxygen documentation patterns for all C files in the OpenLCB library, based on established standards and OpenLCB specifications.

---

## CRITICAL DOCUMENTATION RULES

### ⚠️ RULE #1: NEVER USE @param FOR FUNCTIONS WITHOUT PARAMETERS

**WRONG - DO NOT DO THIS:**
```c
/**
 * @brief Initializes the buffer store
 * @param None          ← WRONG! Remove this line!
 * @return None         ← WRONG! Remove this line!
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
     * Use cases:
     * - Called once during startup
     *
     * @warning MUST be called during initialization
     * @attention Call before any allocation operations
     *
     * @see BufferStore_allocate - Uses initialized pool
     */
void BufferStore_initialize(void);
```

### ⚠️ RULE #2: NEVER INCLUDE BIG-O NOTATION OR COMPLEXITY ANALYSIS

**WRONG - DO NOT DO THIS:**
```c
/**
 * @brief Searches for a message
 *
 * @note Time complexity: O(n)          ← WRONG! Remove this!
 * @remark O(n) worst case              ← WRONG! Remove this!
 * @note Space complexity: O(1)         ← WRONG! Remove this!
 */
openlcb_msg_t* find_message(uint16_t id);
```

**CORRECT:**
```c
/**
 * @brief Searches for a message
 *
 * @note Linear search through all entries
 * @note Non-destructive operation
 *
 * @see add_message - How messages are added to list
 */
openlcb_msg_t* find_message(uint16_t id);
```

**Why no Big-O notation:**
- This is embedded C code, not academic algorithm documentation
- Users care about actual behavior, not theoretical complexity
- Keep documentation practical and focused on usage
- If performance matters, describe it in plain language:
  - "Linear search through all entries" instead of "O(n)"
  - "Constant time lookup" instead of "O(1)"
  - "Searches entire list" instead of "O(n) complexity"

### ⚠️ RULE #3: NEVER INCLUDE ALGORITHM DETAILS IN HEADER FILES

**CRITICAL: Algorithm implementation details MUST ONLY appear in .c files, NEVER in .h files**

**WRONG - DO NOT DO THIS IN HEADER FILES:**
```c
// In .h file - WRONG!
/**
 * @brief Allocates a new buffer
 *
 * @details Algorithm:                   ← WRONG! No "Algorithm:" in headers!
 * -# Search buffer pool for free slot   ← WRONG! No implementation steps!
 * -# Mark buffer as allocated           ← WRONG! No implementation steps!
 * -# Set reference count to 1           ← WRONG! No implementation steps!
 * -# Return pointer to buffer
 *
 * @return Pointer to buffer or NULL
 */
openlcb_msg_t* BufferStore_allocate(void);
```

**CORRECT - HEADER FILE (.h):**
```c
    /**
     * @brief Allocates a new buffer from the pool
     *
     * @details Searches the buffer pool for an available buffer and returns
     * a pointer to it. The buffer is marked as allocated and ready for use.
     *
     * Use cases:
     * - Creating new outgoing messages
     * - Assembling received multi-frame messages
     *
     * @return Pointer to allocated buffer, or NULL if pool is exhausted
     *
     * @warning Returns NULL when pool exhausted - caller MUST check
     * @warning NOT thread-safe
     *
     * @attention Always check return value for NULL
     *
     * @see BufferStore_free - Frees allocated buffer
     */
openlcb_msg_t* BufferStore_allocate(void);
```

**CORRECT - IMPLEMENTATION FILE (.c):**
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
     * Use cases:
     * - Creating new outgoing messages
     * - Assembling received multi-frame messages
     *
     * @return Pointer to allocated buffer, or NULL if pool is exhausted
     *
     * @warning Returns NULL when pool exhausted - caller MUST check
     * @warning NOT thread-safe
     *
     * @attention Always check return value for NULL
     *
     * @see BufferStore_free - Frees allocated buffer
     */
openlcb_msg_t* BufferStore_allocate(void) {
    // Implementation...
}
```

**Key Differences:**

**Header Files (.h) should have:**
- WHAT the function does (high-level purpose)
- WHEN to use it (use cases)
- WHAT it returns and error conditions
- Important warnings and requirements
- Cross-references to related functions

**Header Files (.h) should NOT have:**
- HOW it works (algorithm steps)
- Implementation details
- Step-by-step processing flow
- Internal state changes
- Data structure manipulation details

**Implementation Files (.c) should have:**
- Everything from the header PLUS
- "Algorithm:" section with step-by-step details
- Internal state changes
- Data structure manipulation
- Processing flow details

### ⚠️ RULE #4: DOXYGEN BLOCK IS ALWAYS AT CODE COLUMN + 4

**CRITICAL: The Doxygen block must always be indented 4 spaces MORE than the code
it documents.  The rule is purely relative — find the column where the code starts,
add 4, and put the `/**` there.  File type (.c or .h) does not change this.**

If the code (function definition, declaration, variable, struct member) starts at
column N, the opening `/**` of its Doxygen block starts at column N + 4.

**Examples:**

Code at column 0 → block at column 4:
```c
    /**
     * @brief Allocates a new buffer from the pool
     *
     * @return Pointer to @ref openlcb_msg_t buffer, or NULL if pool exhausted
     *
     * @warning Returns NULL when pool exhausted - caller MUST check
     */
openlcb_msg_t* BufferStore_allocate(void) {
    // Implementation...
}
```

Code at column 4 → block at column 8:
```c
extern "C"
{
        /**
         * @brief Allocates a new buffer from the pool
         *
         * @return Pointer to @ref openlcb_msg_t buffer, or NULL if pool exhausted
         *
         * @warning Returns NULL when pool exhausted - caller MUST check
         */
    openlcb_msg_t* BufferStore_allocate(void);
}
```

Code at column 8 → block at column 12:
```c
            /**
             * @brief Does something at a deeper nesting level.
             */
        extern void SomeModule_nested(void);
```

**WRONG — block at same level as code (no +4 offset):**
```c
    /**                                  <- WRONG! Block at 4 spaces...
     * @brief Allocates a new buffer
     */
    openlcb_msg_t* BufferStore_allocate(void);  <- ...code also at 4 spaces
```

**WRONG — block at column 0 when code is also at column 0:**
```c
/**                                      <- WRONG! No separation from code
 * @brief Allocates a new buffer
 */
openlcb_msg_t* BufferStore_allocate(void) {    <- code also at column 0
```

**Quick lookup:**

| Code column | Block column |
|---|---|
| 0  | 4  |
| 4  | 8  |
| 8  | 12 |
| 12 | 16 |

### ⚠️ RULE #5: NEVER USE DOCUMENTATION SECTIONS WITH PARAMETER NAMES

**CRITICAL: Do NOT mention parameter names (@param) within any @warning, @note, @retval, @exception, @throw, or @attention sections. Doxygen will interpret this as a duplicate @param section and generate errors. **

**Tags affected by this issue:**
- `@warning` - Warning messages
- `@note` - Additional notes
- `@attention` - Required attention points
- `@retval` - Return value descriptions
- `@exception` / `@throw` / `@throws` - Exception descriptions
- `@remark` - Remarks

**WRONG - DO NOT DO THIS:**
```c
/**
 * @brief Registers a new Alias/NodeID pair
 *
 * @param alias The 12-bit CAN alias to store
 * @param node_id The 48-bit OpenLCB Node ID
 *
 * @return Pointer to the registered entry, or NULL if failed
 *
 * @warning Returns NULL if alias is invalid (0 or > 0xFFF)    ← WRONG! "alias" triggers duplicate @param
 * @warning Returns NULL if node_id is invalid                ← WRONG! "node_id" triggers duplicate @param
 * @retval NULL if alias exceeds valid range                  ← WRONG! "alias" triggers duplicate @param
 * @note The alias parameter must be non-zero                 ← WRONG! "alias" triggers duplicate @param
 * @attention msg must not be NULL                            ← WRONG! "msg" triggers duplicate @param
 */
alias_mapping_t *AliasMappings_register(uint16_t alias, node_id_t node_id);
```

**CORRECT:**
```c
    /**
     * @brief Registers a new Alias/NodeID pair
     *
     * @param alias The 12-bit CAN alias to store (valid range: 0x001-0xFFF)
     * @param node_id The 48-bit OpenLCB Node ID to associate with the alias
     *
     * @return Pointer to the registered entry, or NULL if failed
     *
     * @warning Invalid alias values (0 or > 0xFFF) will cause NULL return
     * @warning Invalid node_id values (0 or > 0xFFFFFFFFFFFF) will cause NULL return
     * @warning Returns NULL when buffer is completely full
     * @retval NULL Buffer is full or invalid parameters provided
     * @note Zero values are not permitted for either parameter
     * @attention NULL pointers must be checked before dereferencing return value
     */
alias_mapping_t *AliasMappings_register(uint16_t alias, node_id_t node_id);
```

**Why this matters:**
- Doxygen parsers scan ALL documentation sections for parameter names
- When a parameter name appears at the start of these sections, Doxygen treats it as another @param
- This causes build warnings: "argument 'X' has multiple @param documentation sections"
- The issue affects @warning, @note, @retval, @exception, @throw, @attention, and @remark
- Rephrase to start with the condition or consequence instead
- Put parameter details in the actual @param section, not scattered in other tags

**Safe rephrasing patterns:**
- ❌ "Returns NULL if alias is invalid"
- ✅ "Invalid alias values will cause NULL return"
- ❌ "Returns NULL if node_id exceeds range"
- ✅ "Out-of-range node_id values return NULL"
- ❌ "alias must be non-zero"
- ✅ "Zero values are not permitted"
- ❌ "msg parameter cannot be NULL"
- ✅ "NULL pointers are not permitted"
- ❌ "@retval NULL if buffer is invalid"
- ✅ "@retval NULL Invalid buffer or allocation failure"
- ❌ "@exception IOException if filename is invalid"
- ✅ "@exception IOException Invalid filename provided"

**Quick test:** If your  @warning/@note/@retval/@exception/@throw/@attention, or @remark text could be misread as starting with a parameter name, rephrase it!

### ⚠️ RULE #6: ALWAYS USE @verbatim FOR @param IN .c FILES

**CRITICAL: When function documentation exists in BOTH .h and .c files, Doxygen will detect duplicate @param sections and generate errors. To prevent this, wrap ALL @param tags in the .c file with @verbatim/@endverbatim tags.**

**The Problem:**
- Doxygen parses both header (.h) and implementation (.c) files
- If @param tags exist in both locations, Doxygen reports: "argument 'X' has multiple @param documentation sections"
- This violates Doxygen's rule that each parameter should be documented only once

**The Solution:**
- Keep @param tags in the .h file (where they belong)
- Wrap @param tags in .c files inside @verbatim/@endverbatim blocks
- This makes the .c file @param tags visible to human readers but invisible to Doxygen

**CORRECT - Implementation File (.c):**
```c
    /**
     * @brief Registers a new Alias/NodeID pair
     *
     * @details Algorithm:
     * -# Validate alias is within valid OpenLCB range (0x001-0xFFF)
     * -# Validate node_id is within valid OpenLCB range
     * -# Iterate through buffer looking for empty slot or matching node_id
     * -# Store alias and node_id in found slot
     * -# Return pointer to the entry
     *
     * Use cases:
     * - Registering newly allocated alias during node login
     * - Updating alias after conflict resolution
     *
     * @verbatim
     * @param alias The 12-bit CAN alias to store (valid range: 0x001-0xFFF)
     * @param node_id The 48-bit OpenLCB Node ID to associate with the alias
     * @endverbatim
     *
     * @return Pointer to registered entry, or NULL if buffer full or invalid parameters
     *
     * @warning Invalid alias values (0 or > 0xFFF) will cause NULL return
     * @warning Invalid node_id values will cause NULL return
     *
     * @see AliasMappings_unregister - Removes a mapping
     */
alias_mapping_t *AliasMappings_register(uint16_t alias, node_id_t node_id) {
    // Implementation...
}
```

**WRONG - DO NOT DO THIS IN .c FILES:**
```c
/**
 * @brief Registers a new Alias/NodeID pair
 *
 * @details Algorithm:
 * -# Validate and process...
 *
 * @param alias The 12-bit CAN alias          ← WRONG! Not wrapped in @verbatim
 * @param node_id The 48-bit Node ID          ← WRONG! Doxygen will see duplicate @param
 *
 * @return Pointer to registered entry
 */
alias_mapping_t *AliasMappings_register(uint16_t alias, node_id_t node_id) {
    // Implementation...
}
```

**Why @verbatim/@endverbatim works:**
- @verbatim tells Doxygen: "Don't parse this text as commands"
- The @param tags become plain text to Doxygen
- Human readers can still see the parameter documentation in the .c file
- Doxygen only processes the @param tags from the .h file
- No duplicate @param warnings are generated

**Placement Guidelines:**
- Place @verbatim block AFTER @details and algorithm description
- Place @verbatim block BEFORE @return tag
- Keep the visual structure identical to normal @param tags for readability
- The @verbatim block can include the entire parameter section

**Alternative approach (NOT recommended):**
- Remove ALL @param tags from .c files entirely
- Only document parameters in .h files
- Drawback: Developers reading .c file don't see parameter docs

**Why we use @verbatim instead of removal:**
- Keeps parameter documentation visible in both files
- Easier for developers who work primarily in .c files
- Self-documenting code remains self-documenting
- No need to switch between .h and .c files to understand parameters

**Important placement rule:**
```c
/**
 * @brief Function description
 *
 * @details Algorithm details here...
 *
 * @verbatim
 * @param x First parameter        ← Inside verbatim
 * @param y Second parameter       ← Inside verbatim  
 * @endverbatim
 *
 * @return Description              ← Outside verbatim (Doxygen processes this)
 *
 * @warning Important notes         ← Outside verbatim (Doxygen processes this)
 */
```

### Rules for @param and @return tags:

1. **If function has NO parameters** → DO NOT include any @param tags
2. **If function returns void** → DO NOT include @return tag
3. **If function has parameters** → Include @param for EACH parameter
4. **If function returns non-void** → Include @return tag with description
5. **If documenting in BOTH .h and .c** → Wrap .c file @param in @verbatim/@endverbatim

### Examples:

```c
// Function with NO parameters, returns void
void initialize(void);
// Documentation: NO @param, NO @return

// Function with NO parameters, returns value
uint16_t get_count(void);
// Documentation: NO @param, YES @return

// Function with parameters, returns void
void set_value(uint16_t value);
// Documentation: YES @param, NO @return

// Function with parameters, returns value
uint16_t add(uint16_t a, uint16_t b);
// Documentation: YES @param (for both a and b), YES @return
```

---

### ⚠️ RULE #10: HEADER EXTERN DECLARATIONS MUST INCLUDE @param WITH TYPE LINKS

**CRITICAL: All `extern` function declarations and `extern` variable declarations in header files MUST include `@param` tags for every parameter.  Any parameter whose type is defined by the library (a typedef, struct, or enum from the project) MUST use `@ref` to link to that type.**

**This rule does NOT apply to interface struct function-pointer members.**  Callback pointers inside `typedef struct { ... } interface_xxx_t;` structs use Size 1 inline doc (a single `/** @brief ... */` line).  They do not need `@param` tags — the brief is sufficient.

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

**CORRECT — extern function with multiple params:**
```c
        /**
         * @brief Build outgoing Lock/Reserve command datagram.
         *
         * @param statemachine_info                   Pointer to @ref openlcb_statemachine_info_t context.
         * @param config_mem_operations_request_info   Pointer to @ref config_mem_operations_request_info_t request.
         */
    extern void ProtocolConfigMemOperationsHandler_request_reserve_lock(
            openlcb_statemachine_info_t *statemachine_info,
            config_mem_operations_request_info_t *config_mem_operations_request_info);
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
            openlcb_statemachine_info_t *statemachine_info);   // ← WRONG: no @param!
```

**WRONG — @param present but missing @ref for library type:**
```c
        /**
         * @brief Handle incoming Get Address Space Info command.
         *
         * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
         *                            ← WRONG: needs @ref openlcb_statemachine_info_t
         */
```

**When to use @ref:**
- Use `@ref TypeName` for any type defined in the library (`_t` typedefs, `_enum` enums, struct tags)
- Do NOT use `@ref` for standard C types (`uint8_t`, `uint16_t`, `bool`, `int`, `void`, etc.)
- Do NOT use `@ref` for types from standard headers (`size_t`, `FILE`, etc.)

**Summary:**

| Location | @param required? | @ref for library types? |
|---|---|---|
| `extern` function in `.h` | YES | YES |
| `extern` variable in `.h` | N/A (no params) | YES in @brief if type is library-defined |
| Interface struct callback member | NO (Size 1 brief only) | NO |
| Function definition in `.c` | YES (wrapped in @verbatim) | YES |

---

### ⚠️ RULE #7: NEVER CHANGE THE NAMES OF ANY CODE IDENTIFIERS WITHOUT ASKING

**CRITICAL: In ALL files (.c AND .h), Do not change constants, function names, variables, etc. without a discussion**

### ⚠️ RULE #8: ALWAYS UPDATE THE @date IN THE FILE HEADER WHEN MODIFYING A FILE

**CRITICAL: Every time a .c or .h file is modified, the @date field in the file header MUST be updated to the current date.**

The `@date` field records when the file was last changed, not when it was originally created.  Leaving a stale date makes it impossible to tell at a glance which files have been recently worked on.

**WRONG - stale date after modification:**
```c
/**
 * @file alias_mappings.h
 * @brief Fixed-size buffer mapping CAN aliases to Node IDs.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026        ← WRONG if file was modified later
 */
```

**CORRECT - date updated on modification:**
```c
/**
 * @file alias_mappings.h
 * @brief Fixed-size buffer mapping CAN aliases to Node IDs.
 *
 * @author Jim Kueneman
 * @date 27 Feb 2026        ← Updated to the date the file was changed
 */
```

**Rules:**
- Update `@date` in BOTH the `.h` and `.c` file whenever either is modified
- Use the format: DD Mon YYYY (e.g. 27 Feb 2026)
- The `@date` appears ONLY in the file header — not in individual function blocks
- When running a documentation update pass over multiple files, each file gets today's date

## Documentation Standards Summary

### Required Tags
- **@file** - File name and brief description
- **@brief** - One-line summary
- **@details** - Detailed explanation (NO "Algorithm:" in .h, YES "Algorithm:" in .c)
- **@author** - Jim Kueneman
- **@date** - 17 Jan 2026
- **@param** - Parameter descriptions (ONLY if function has parameters)
- **@return** - Return value descriptions (ONLY if function returns non-void)
- **@warning** - Critical dangers (NULL crashes, buffer overflows)
- **@attention** - Important requirements (calling order, dependencies)
- **@note** - Helpful information
- **@remark** - Performance notes (NO Big-O notation)

### Algorithm Documentation Pattern

**For Header Files (.h):**
```c
    /**
     * @brief Brief one-line description
     * 
     * @details High-level explanation of WHAT the function does and WHEN to use it.
     * NO algorithm steps, NO implementation details.
     * 
     * Use cases:
     * - Use case 1
     * - Use case 2
     * 
     * @param param1 Description of first parameter (ONLY if parameters exist)
     * @param param2 Description of second parameter (ONLY if parameters exist)
     * @return Description of return value, NULL conditions (ONLY if non-void return)
     * 
     * @warning Critical safety issue that could cause crashes
     * @attention Important requirement for correct usage
     * @note Helpful additional information
     * 
     * @see related_function - Brief description of relationship
     */
return_type function_name(param_type param1, param_type param2);
```

**Note: declaration is at 4 spaces inside `extern "C"`, so block is at 8 spaces (4 + 4).**

**For Implementation Files (.c):**
```c
    /**
     * @brief Brief one-line description
     * 
     * @details Algorithm:
     * Detailed explanation of HOW the function implements its behavior.
     * -# Step 1: First action
     * -# Step 2: Second action
     * -# Step 3: Third action
     * 
     * Use cases:
     * - Use case 1
     * - Use case 2
     * 
     * @param param1 Description of first parameter (ONLY if parameters exist)
     * @param param2 Description of second parameter (ONLY if parameters exist)
     * @return Description of return value, NULL conditions (ONLY if non-void return)
     * 
     * @warning Critical safety issue that could cause crashes
     * @attention Important requirement for correct usage
     * @note Helpful additional information
     * 
     * @see related_function - Brief description of relationship
     */
return_type function_name(param_type param1, param_type param2) {
    // Implementation...
}
```

**Note: function definition is at column 0, so block is at 4 spaces (0 + 4).**

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
     * Use cases:
     * - Called once during application startup
     * - Required before any buffer allocation operations
     *
     * @warning MUST be called exactly once during initialization
     * @warning NOT thread-safe
     *
     * @attention Call before OpenLcbBufferFifo_initialize()
     *
     * @see OpenLcbBufferStore_allocate_buffer - Allocates from initialized pool
     */
void OpenLcbBufferStore_initialize(void);
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
     * Use cases:
     * - Called once during application startup
     * - Required before any buffer allocation operations
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
     * Use cases:
     * - Runtime monitoring of buffer usage
     * - Detecting buffer leaks
     * - Load balancing decisions
     *
     * @return Number of BASIC sized messages currently allocated (0 to USER_DEFINED_BASIC_BUFFER_DEPTH)
     *
     * @note This is a live count that changes as buffers are allocated and freed
     *
     * @see OpenLcbBufferStore_basic_messages_max_allocated - Peak usage counter
     */
uint16_t OpenLcbBufferStore_basic_messages_allocated(void);
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
     * Use cases:
     * - Runtime monitoring of buffer usage
     * - Detecting buffer leaks
     * - Load balancing decisions
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
     * Use cases:
     * - Sharing a buffer between transmit and retry queues
     * - Holding a buffer in multiple lists simultaneously
     *
     * @param msg Pointer to message buffer to increment reference count
     *
     * @warning NULL pointers will cause undefined behavior - no NULL check performed
     * @warning NOT thread-safe
     *
     * @attention Always pair with corresponding free_buffer() call
     *
     * @see OpenLcbBufferStore_free_buffer - Decrements reference count
     */
void OpenLcbBufferStore_inc_reference_count(openlcb_msg_t *msg);
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
     * Use cases:
     * - Sharing a buffer between transmit and retry queues
     * - Holding a buffer in multiple lists simultaneously
     *
     * @verbatim
     * @param msg Pointer to message buffer to increment reference count
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
     * Use cases:
     * - Creating new outgoing OpenLCB messages
     * - Assembling multi-frame received messages
     *
     * @param payload_type Type of buffer requested (BASIC, DATAGRAM, SNIP, or STREAM)
     * @return Pointer to allocated message buffer, or NULL if pool exhausted or invalid type
     *
     * @warning Returns NULL when buffer pool is exhausted - caller MUST check for NULL
     * @warning NOT thread-safe
     *
     * @attention Always check return value for NULL before use
     *
     * @see OpenLcbBufferStore_free_buffer - Frees allocated buffer
     */
openlcb_msg_t* OpenLcbBufferStore_allocate_buffer(payload_type_enum payload_type);
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
     * Use cases:
     * - Creating new outgoing OpenLCB messages
     * - Assembling multi-frame received messages
     *
     * @verbatim
     * @param payload_type Type of buffer requested (BASIC, DATAGRAM, SNIP, or STREAM)
     * @endverbatim
     *
     * @return Pointer to allocated message buffer, or NULL if pool exhausted or invalid type
     *
     * @warning Buffer pool exhaustion returns NULL - caller MUST check for NULL
     * @warning NOT thread-safe
     *
     * @attention Always check return value for NULL before use
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
- [ ] **Doxygen block at declaration column + 4 (e.g. declaration at 4 → block at 8)**
- [ ] @brief with one-line summary
- [ ] @details with WHAT and WHEN (NO HOW/Algorithm)
- [ ] Use cases list
- [ ] @param only if function has parameters
- [ ] @return only if function returns non-void
- [ ] @warning for critical issues (NEVER start with parameter names!)
- [ ] @attention for requirements (NEVER start with parameter names!)
- [ ] @note for helpful info (NEVER start with parameter names!)
- [ ] @retval for return values (NEVER start with parameter names!)
- [ ] @see for cross-references
- [ ] NO "Algorithm:" section
- [ ] NO implementation steps
- [ ] NO Big-O notation
- [ ] NO parameter names at start of @warning, @note, @retval, @exception, @throw, @attention, or @remark

**Implementation Files (.c) Documentation Must Have:**
- [ ] **Doxygen block at function column + 4 (e.g. function at column 0 → block at 4)**
- [ ] @brief with one-line summary
- [ ] @details with "Algorithm:" and step-by-step HOW
- [ ] Use cases list
- [ ] **@param MUST be wrapped in @verbatim/@endverbatim if also documented in .h file**
- [ ] @return only if function returns non-void
- [ ] @warning for critical issues (NEVER start with parameter names!)
- [ ] @attention for requirements (NEVER start with parameter names!)
- [ ] @note for helpful info (NEVER start with parameter names!)
- [ ] @retval for return values (NEVER start with parameter names!)
- [ ] @see for cross-references
- [ ] YES "Algorithm:" section with numbered steps
- [ ] YES implementation details
- [ ] NO Big-O notation
- [ ] NO parameter names at start of @warning, @note, @retval, @exception, @throw, @attention, or @remark
- [ ] NO bare @param tags (always wrap in @verbatim when .h file has @param)

**Remember:**
- Headers describe WHAT and WHEN (with 4-space indentation)
- Implementations describe HOW (with 4-space indentation)
- Never mix algorithm details into headers
- Never use Big-O notation anywhere
- Never use @param None or @return None
- Never start @warning, @note, @retval, @exception, @throw, @attention, or @remark with parameter names
- **ALWAYS wrap @param in @verbatim/@endverbatim in .c files when .h file has @param**
- Doxygen block is always at code column + 4 — the rule is relative, not fixed to any particular column
- **ALWAYS update @date in the file header to the current date when modifying a file**
- Use `@ref TypeName` for any project-defined type appearing in `@param` or `@return`
