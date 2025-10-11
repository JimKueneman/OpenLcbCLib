#include "test/main_Test.hxx"

#include "can_login_message_handler.h"

#include "can_types.h"
#include "can_utilities.h"
#include "can_rx_message_handler.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_buffer_list.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_utilities.h"
#include "../../openlcb/openlcb_node.h"

#define NODE_ID 0x010203040506
#define ALIAS 0xAAA
