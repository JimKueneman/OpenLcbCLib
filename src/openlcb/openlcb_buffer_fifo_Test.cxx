#include "test/main_Test.hxx"

#include "openlcb_buffer_fifo.h"
#include "openlcb_types.h"

TEST(OpenLcbBufferFIFO, initialize)
{

    BufferFifo_initialize();

    openlcb_msg_t *openlcb_msg = BufferFifo_push(DATAGRAM);

    EXPECT_NE(openlcb_msg, nullptr);
}

TEST(OpenLcbBufferFIFO, allocate_basic_buffer)
{

    BufferFifo_initialize();

    openlcb_msg_t *openlcb_msg = BufferFifo_push(BASIC);

    EXPECT_NE(openlcb_msg, nullptr);
}

TEST(OpenLcbBufferFIFO, allocate_datagram_buffer)
{

    BufferFifo_initialize();

    openlcb_msg_t *openlcb_msg = BufferFifo_push(DATAGRAM);

    EXPECT_NE(openlcb_msg, nullptr);
}

TEST(OpenLcbBufferFIFO, allocate_snip_buffer)
{

    BufferFifo_initialize();

    openlcb_msg_t *openlcb_msg = BufferFifo_push(SNIP);

    EXPECT_NE(openlcb_msg, nullptr);
}

TEST(OpenLcbBufferFIFO, allocate_stream_buffer)
{

    // Streams not implemented yet 7.29.25

    // BufferFifo_initialize();

    // openlcb_msg_t *openlcb_msg = BufferFifo_push(STREAM);

    // EXPECT_NE(openlcb_msg, nullptr);
}