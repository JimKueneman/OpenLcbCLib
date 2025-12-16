#include "test/main_Test.hxx"

#include "can_types.h"

#include "alias_mappings.h"

#include "../../openlcb/openlcb_types.h"

#define NODE_ID 0x010203040506
#define NODE_ALIAS 0x0666

void _global_initialize(void)
{

    // CanBufferStore_initialize();
    // CanBufferFifo_initialize();
    // OpenLcbBufferStore_initialize();
    // OpenLcbBufferFifo_initialize();
    // OpenLcbBufferList_initialize();
    // OpenLcbNode_initialize(&interface_openlcb_node);

    AliasMappings_initialize();
}

void _reset_variables(void)
{
}

TEST(AliasMapping, initialize)
{

    _global_initialize();
    _reset_variables();
}

TEST(AliasMapping, get_alias_mapping_info)
{

    _global_initialize();
    _reset_variables();

    alias_mapping_info_t *alias_mapping_info = AliasMappings_get_alias_mapping_info();

    EXPECT_NE(alias_mapping_info, nullptr);
}

TEST(AliasMapping, set_has_duplicate_alias_flag)
{

    _global_initialize();
    _reset_variables();

    AliasMappings_set_has_duplicate_alias_flag();

    alias_mapping_info_t *alias_mapping_info = AliasMappings_get_alias_mapping_info();

    EXPECT_TRUE(alias_mapping_info->has_duplicate_alias);
}

TEST(AliasMapping, register)
{

    _global_initialize();
    _reset_variables();

    alias_mapping_t *mapping = nullptr;

    for (int i = 0; i < USER_DEFINED_ALIAS_MAPPING_BUFFER_DEPTH; i++)
    {

        mapping = AliasMappings_register(NODE_ALIAS + i, NODE_ID + i);
        EXPECT_NE(mapping, nullptr);
    }

    mapping = AliasMappings_register(NODE_ALIAS - 1, NODE_ID - 1);
    EXPECT_EQ(mapping, nullptr);

    for (int i = 0; i < USER_DEFINED_ALIAS_MAPPING_BUFFER_DEPTH; i++)
    {

        mapping = AliasMappings_find_mapping_by_alias(NODE_ALIAS + i);
        EXPECT_NE(mapping, nullptr);
    }

    for (int i = 0; i < USER_DEFINED_ALIAS_MAPPING_BUFFER_DEPTH; i++)
    {

        mapping = AliasMappings_find_mapping_by_node_id(NODE_ID + i);
        EXPECT_NE(mapping, nullptr);
    }

    for (int i = 0; i < USER_DEFINED_ALIAS_MAPPING_BUFFER_DEPTH; i++)
    {

        AliasMappings_unregister(NODE_ALIAS + i);
    }

    for (int i = 0; i < USER_DEFINED_ALIAS_MAPPING_BUFFER_DEPTH; i++)
    {

        mapping = AliasMappings_find_mapping_by_alias(NODE_ALIAS + i);
        EXPECT_EQ(mapping, nullptr);
    }
}

TEST(AliasMapping, register_duplicate)
{

    _global_initialize();
    _reset_variables();

    alias_mapping_t *mapping = nullptr;

    mapping = AliasMappings_register(NODE_ALIAS, NODE_ID);

    mapping = AliasMappings_find_mapping_by_alias(NODE_ALIAS);
    EXPECT_NE(mapping, nullptr);

    // New alias associated with the Node ID but we don't unregister first.  Should just update with the new Alias
    mapping = AliasMappings_register(NODE_ALIAS + 1, NODE_ID);

    mapping = AliasMappings_find_mapping_by_alias(NODE_ALIAS);
    EXPECT_EQ(mapping, nullptr);

    mapping = AliasMappings_find_mapping_by_alias(NODE_ALIAS + 1);
    EXPECT_NE(mapping, nullptr);
}

TEST(AliasMapping, register_misses)
{

    _global_initialize();
    _reset_variables();

    alias_mapping_t *mapping = nullptr;

    mapping = AliasMappings_register(NODE_ALIAS, NODE_ID);

    mapping = AliasMappings_find_mapping_by_alias(NODE_ALIAS + 1);
    EXPECT_EQ(mapping, nullptr);

    mapping = AliasMappings_find_mapping_by_node_id(NODE_ID + 1);
    EXPECT_EQ(mapping, nullptr);

    AliasMappings_unregister(NODE_ALIAS + 1);
    EXPECT_EQ(mapping, nullptr);
}
