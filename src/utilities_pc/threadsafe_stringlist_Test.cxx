/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file threadsafe_stringlist_Test.cxx
 * @brief Unit tests for the ThreadSafeStringList FIFO.
 */

#include <gtest/gtest.h>
#include <cstdlib>

#include "utilities_pc/threadsafe_stringlist.h"

class ThreadSafeStringListTest : public ::testing::Test {

protected:

    StringList list;

    void SetUp() override {

        ThreadSafeStringList_init(&list);

    }

};

// ============================================================================
// Empty FIFO behaviour
// ============================================================================

TEST_F(ThreadSafeStringListTest, PopOnEmptyReturnsNull) {

    EXPECT_EQ(nullptr, ThreadSafeStringList_pop(&list));

}

// ============================================================================
// Push / pop round trip
// ============================================================================

TEST_F(ThreadSafeStringListTest, PushReturnsTrueOnSuccess) {

    EXPECT_EQ(true, ThreadSafeStringList_push(&list, "hello"));

}

TEST_F(ThreadSafeStringListTest, PushedStringIsCopied) {

    char original[] = "test string";
    ThreadSafeStringList_push(&list, original);

    char *popped = ThreadSafeStringList_pop(&list);
    ASSERT_NE(nullptr, popped);
    EXPECT_STREQ("test string", popped);
    EXPECT_NE(original, popped); // must be a distinct heap copy
    free(popped);

}

TEST_F(ThreadSafeStringListTest, PopEmptiesAfterSinglePush) {

    ThreadSafeStringList_push(&list, "only");
    char *first = ThreadSafeStringList_pop(&list);
    ASSERT_NE(nullptr, first);
    free(first);

    EXPECT_EQ(nullptr, ThreadSafeStringList_pop(&list));

}

// ============================================================================
// FIFO ordering
// ============================================================================

TEST_F(ThreadSafeStringListTest, FifoOrderPreserved) {

    ThreadSafeStringList_push(&list, "first");
    ThreadSafeStringList_push(&list, "second");
    ThreadSafeStringList_push(&list, "third");

    char *a = ThreadSafeStringList_pop(&list);
    char *b = ThreadSafeStringList_pop(&list);
    char *c = ThreadSafeStringList_pop(&list);

    ASSERT_NE(nullptr, a);
    ASSERT_NE(nullptr, b);
    ASSERT_NE(nullptr, c);

    EXPECT_STREQ("first",  a);
    EXPECT_STREQ("second", b);
    EXPECT_STREQ("third",  c);

    free(a);
    free(b);
    free(c);

    EXPECT_EQ(nullptr, ThreadSafeStringList_pop(&list));

}

// ============================================================================
// Interleaved push / pop
// ============================================================================

TEST_F(ThreadSafeStringListTest, InterleavedPushPop) {

    ThreadSafeStringList_push(&list, "a");
    ThreadSafeStringList_push(&list, "b");

    char *first = ThreadSafeStringList_pop(&list);
    ASSERT_NE(nullptr, first);
    EXPECT_STREQ("a", first);
    free(first);

    ThreadSafeStringList_push(&list, "c");

    char *second = ThreadSafeStringList_pop(&list);
    ASSERT_NE(nullptr, second);
    EXPECT_STREQ("b", second);
    free(second);

    char *third = ThreadSafeStringList_pop(&list);
    ASSERT_NE(nullptr, third);
    EXPECT_STREQ("c", third);
    free(third);

    EXPECT_EQ(nullptr, ThreadSafeStringList_pop(&list));

}

// ============================================================================
// Capacity limit
// ============================================================================

TEST_F(ThreadSafeStringListTest, PushReturnsFalseWhenFull) {

    // Fill to capacity — the circular buffer holds MAX_STRINGS - 1 entries
    int pushed = 0;
    for (int i = 0; i < MAX_STRINGS; i++) {

        if (ThreadSafeStringList_push(&list, "x"))
            pushed++;

    }

    EXPECT_EQ(MAX_STRINGS - 1, pushed);

    // One more push must fail
    EXPECT_EQ(false, ThreadSafeStringList_push(&list, "overflow"));

}

TEST_F(ThreadSafeStringListTest, CanRefillAfterDraining) {

    // Fill then drain
    for (int i = 0; i < MAX_STRINGS - 1; i++)
        ThreadSafeStringList_push(&list, "x");

    char *s;
    while ((s = ThreadSafeStringList_pop(&list)) != nullptr)
        free(s);

    // Should be able to push again
    EXPECT_EQ(true, ThreadSafeStringList_push(&list, "after drain"));

    char *result = ThreadSafeStringList_pop(&list);
    ASSERT_NE(nullptr, result);
    EXPECT_STREQ("after drain", result);
    free(result);

}

// ============================================================================
// Empty string
// ============================================================================

TEST_F(ThreadSafeStringListTest, EmptyStringRoundTrip) {

    ThreadSafeStringList_push(&list, "");
    char *result = ThreadSafeStringList_pop(&list);
    ASSERT_NE(nullptr, result);
    EXPECT_STREQ("", result);
    free(result);

}
