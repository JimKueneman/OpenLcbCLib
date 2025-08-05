#include "test/main_Test.hxx"

#include "can_frame_message_handler.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_node.h"

static bool _try_transmit_can_message(can_msg_t *can_msg)
{

    return true;
}

const static can_frame_message_handler_interface_t _interface = {

    .try_transmit_can_message = &_try_transmit_can_message};

TEST(CanFrameMessageHandler, cid)
{

    CanFrameMessageHandler_initialize(&_interface);

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg, nullptr);

    openlcb_node_t *openlcb_node = OpenLcbNode_allocate();

    if (openlcb_msg)
    {

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(CanFrameMessageHandler, rid)
{
}

TEST(CanFrameMessageHandler, amd)
{
}

TEST(CanFrameMessageHandler, ame)
{
}

TEST(CanFrameMessageHandler, amr)
{
}
