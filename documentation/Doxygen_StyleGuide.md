# OpenLCB C Library — Doxygen Style Guide

**Version:** 1.2
**Date:** 27 Feb 2026
**Purpose:** Practical rules for writing Doxygen comments that are useful without drowning the code

---

## The Problem This Guide Solves

The goal of documentation is to help, not to hide the code.  A huge block of Doxygen
comments above a two-line function makes the file harder to read, not easier.  When
you open a `.c` or `.h` file you should be able to scan down it and find the functions
quickly.  Comments that go on for 40 lines before a 3-line function work against that.

**The rule of thumb:** documentation length should be roughly proportional to function
complexity.  A one-liner getter needs a one-liner comment.  A complex state-machine
function earns a full block.

For the complete technical rules on Doxygen tag usage (which tags are required, how to
handle `@param` in `.c` vs `.h` files, what NOT to put in headers, etc.) see:

> `documentation/C_Documentation_Guide_v1_7.md`

This guide covers the **style** — how much to write and what it should look like.

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
`C_Documentation_Guide_v1_7.md` Rule #3 for details).

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

**Do not use Big-O notation.**  Write in plain language.

---

## Linking to Custom Types

When a `@param` or `@return` involves a custom type (any `typedef`, `struct`, or
`enum` defined in the project — not a standard C type like `uint16_t`), use
`@ref` to create a clickable link to that type's documentation in the generated
HTML.

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
- In `@see` tags, prefer `@ref` for types and plain function names for functions.

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
 * @date 27 Feb 2026
 */
```

Three to five sentences is enough.  Do NOT repeat `@author` and `@date` in the
function blocks — they belong in the file header only.

**Always update `@date` to the current date whenever a file is modified.**

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

## Indentation

**The Doxygen block is always indented 4 spaces more than the code it documents.**

That is the entire rule.  It applies everywhere, in every file type.

Find the first non-space character of the line being documented.  Count how many
spaces precede it (call that N).  The opening `/**` of the Doxygen block goes at
column N + 4.

**Examples:**

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

The relationship is always: **block column = code column + 4**.  The specific column
numbers depend on where the code sits, not on whether you are in a `.c` or `.h` file.

**Quick lookup:**

| Code starts at column | Doxygen block starts at column |
|---|---|
| 0  | 4  |
| 4  | 8  |
| 8  | 12 |
| 12 | 16 |

---

## Critical Rules Summary

The full rules are in `C_Documentation_Guide_v1_7.md`.  The ones that cause the most
Doxygen build warnings are:

1. No `@param` for parameter-less functions.
2. No Big-O notation anywhere.
3. No algorithm steps in `.h` files — only in `.c` files.
4. **Doxygen block is always at code column + 4.**  Code at column 0 → block at 4.
   Code at column 4 → block at 8.  Code at column 8 → block at 12.
   The rule is relative — it does not depend on file type.
5. Never start `@warning`, `@note`, `@retval`, `@attention`, or `@remark` text with a
   parameter name — Doxygen reads it as a duplicate `@param`.
6. Wrap `@param` tags inside `@verbatim`/`@endverbatim` in `.c` files when the `.h`
   file already documents the same parameters.
7. Never rename any identifier without a discussion first.
8. Always update `@date` in the file header to today's date when modifying a file.
9. Use `@ref TypeName` for every project-defined custom type that appears in a
   `@param` or `@return` description.  Do not use `@ref` for standard C types.
10. All `extern` function declarations in `.h` files **must** include `@param` tags
    (with `@ref` links for library-defined types).  Interface struct callback
    members are exempt — they use Size 1 inline briefs only.

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
