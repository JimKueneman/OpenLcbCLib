#include "test/main_Test.hxx"

#include "protocol_message_network.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"

bool lock_node_list_called = false;
bool unlock_node_list_called = false;

void lock_node_list(void)
{

    lock_node_list_called = true;
}

void unlock_node_list(void)
{

    unlock_node_list_called = true;
}

interface_openlcb_protocol_message_network_t interface_openlcb_protocol_message_network = {

};

interface_openlcb_node_t interface_openlcb_node = {

    .lock_node_list = &lock_node_list,
    .unlock_node_list = &unlock_node_list};

void _reset_variables(void)
{

    lock_node_list_called = false;
    unlock_node_list_called = false;
}

void _global_initialize(void)
{

    ProtocolMessageNetwork_initialize(&interface_openlcb_protocol_message_network);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

TEST(ProtocolMessageNetowrk, initialize)
{

    _reset_variables();
    _global_initialize();
}