#include "test/main_Test.hxx"

#include "openlcb_buffer_fifo.h"
#include "openlcb_types.h"

TEST(OpenLcbBufferFIFO, initialize)
{

    //  BufferFifo_initialize();

    openlcb_msg_t *openlcb_msg = BufferFifo_push(DATAGRAM);

    EXPECT_NE(openlcb_msg, nullptr);
}