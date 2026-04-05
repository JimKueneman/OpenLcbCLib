/*******************************************************************************
 * File: tcp_mdns_Test.cxx
 *
 * Description:
 *   Test suite for TCP mDNS module — verifies mDNS discovery and advertisement
 *   through the platform-specific DI backend.
 *
 * Module Under Test:
 *   TcpMdns — mDNS service discovery and registration
 *
 * Test Coverage:
 *   - discover_hub calls mdns_query with correct service type
 *   - advertise_hub calls mdns_register with hub service type
 *   - advertise_node calls mdns_register with node service type
 *   - stop_advertising calls mdns_unregister
 *   - All functions return false / no-op before initialize
 *   - Callback passed through to mdns_query
 *
 * Author: Test Suite
 * Date: 2026-04-05
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "tcp_mdns.h"

#include <string.h>

// =============================================================================
// Mock state
// =============================================================================

static bool _query_called = false;
static char _query_service_type[128];
static tcp_mdns_result_callback_t _query_callback = NULL;

static bool _register_called = false;
static char _register_service_type[128];
static char _register_name[128];
static uint16_t _register_port = 0;

static bool _unregister_called = false;
static char _unregister_name[128];

// =============================================================================
// Mock functions
// =============================================================================

static bool _mock_mdns_query(const char *service_type, tcp_mdns_result_callback_t callback)
{
    _query_called = true;
    strncpy(_query_service_type, service_type, sizeof(_query_service_type) - 1);
    _query_callback = callback;
    return true;
}

static bool _mock_mdns_register(const char *service_type, const char *name, uint16_t port)
{
    _register_called = true;
    strncpy(_register_service_type, service_type, sizeof(_register_service_type) - 1);
    strncpy(_register_name, name, sizeof(_register_name) - 1);
    _register_port = port;
    return true;
}

static void _mock_mdns_unregister(const char *name)
{
    _unregister_called = true;
    strncpy(_unregister_name, name, sizeof(_unregister_name) - 1);
}

// =============================================================================
// Dummy discovery callback
// =============================================================================

static bool _discovery_callback_called = false;

static void _dummy_discovery_callback(const tcp_mdns_result_t *result)
{
    _discovery_callback_called = true;
    (void)result;
}

// =============================================================================
// Test setup
// =============================================================================

static const tcp_mdns_config_t _mdns_config = {
    .mdns_query      = &_mock_mdns_query,
    .mdns_register   = &_mock_mdns_register,
    .mdns_unregister = &_mock_mdns_unregister,
};

static void reset_mocks(void)
{
    _query_called = false;
    memset(_query_service_type, 0, sizeof(_query_service_type));
    _query_callback = NULL;

    _register_called = false;
    memset(_register_service_type, 0, sizeof(_register_service_type));
    memset(_register_name, 0, sizeof(_register_name));
    _register_port = 0;

    _unregister_called = false;
    memset(_unregister_name, 0, sizeof(_unregister_name));

    _discovery_callback_called = false;
}

static void setup_test(void)
{
    reset_mocks();
    TcpMdns_initialize(&_mdns_config);
}

// =============================================================================
// Discover Hub
// =============================================================================

TEST(TCP_Mdns, discover_hub_calls_query_with_hub_service_type)
{
    setup_test();

    bool result = TcpMdns_discover_hub(&_dummy_discovery_callback);

    EXPECT_TRUE(result);
    EXPECT_TRUE(_query_called);
    EXPECT_STREQ(_query_service_type, TCP_MDNS_HUB_SERVICE_TYPE);
}

TEST(TCP_Mdns, discover_hub_passes_callback)
{
    setup_test();

    TcpMdns_discover_hub(&_dummy_discovery_callback);

    EXPECT_EQ(_query_callback, &_dummy_discovery_callback);
}

// =============================================================================
// Advertise Hub
// =============================================================================

TEST(TCP_Mdns, advertise_hub_registers_with_hub_service_type)
{
    setup_test();

    bool result = TcpMdns_advertise_hub("MyHub", 12021);

    EXPECT_TRUE(result);
    EXPECT_TRUE(_register_called);
    EXPECT_STREQ(_register_service_type, TCP_MDNS_HUB_SERVICE_TYPE);
    EXPECT_STREQ(_register_name, "MyHub");
    EXPECT_EQ(_register_port, 12021);
}

// =============================================================================
// Advertise Node
// =============================================================================

TEST(TCP_Mdns, advertise_node_registers_with_node_service_type)
{
    setup_test();

    bool result = TcpMdns_advertise_node("MyNode", 12022);

    EXPECT_TRUE(result);
    EXPECT_TRUE(_register_called);
    EXPECT_STREQ(_register_service_type, TCP_MDNS_NODE_SERVICE_TYPE);
    EXPECT_STREQ(_register_name, "MyNode");
    EXPECT_EQ(_register_port, 12022);
}

// =============================================================================
// Stop Advertising
// =============================================================================

TEST(TCP_Mdns, stop_advertising_calls_unregister)
{
    setup_test();

    TcpMdns_stop_advertising("MyHub");

    EXPECT_TRUE(_unregister_called);
    EXPECT_STREQ(_unregister_name, "MyHub");
}

// =============================================================================
// Before Initialize — Returns False / No-op
// =============================================================================

TEST(TCP_Mdns, discover_before_init_returns_false)
{
    // Re-initialize with NULL config to simulate uninitialized state
    TcpMdns_initialize(NULL);
    reset_mocks();

    bool result = TcpMdns_discover_hub(&_dummy_discovery_callback);

    EXPECT_FALSE(result);
    EXPECT_FALSE(_query_called);
}

TEST(TCP_Mdns, advertise_hub_before_init_returns_false)
{
    TcpMdns_initialize(NULL);
    reset_mocks();

    bool result = TcpMdns_advertise_hub("MyHub", 12021);

    EXPECT_FALSE(result);
    EXPECT_FALSE(_register_called);
}

TEST(TCP_Mdns, advertise_node_before_init_returns_false)
{
    TcpMdns_initialize(NULL);
    reset_mocks();

    bool result = TcpMdns_advertise_node("MyNode", 12022);

    EXPECT_FALSE(result);
    EXPECT_FALSE(_register_called);
}

TEST(TCP_Mdns, stop_advertising_before_init_no_crash)
{
    TcpMdns_initialize(NULL);
    reset_mocks();

    TcpMdns_stop_advertising("MyHub");

    // No crash = pass
    EXPECT_FALSE(_unregister_called);
}
