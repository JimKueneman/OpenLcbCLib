#include "test/main_Test.hxx"

#include "openlcb_gridconnect.h"

#include "openlcb_types.h"

TEST(OpenLcbGridConnect, copy_out_gridconnect_when_done)
{

    gridconnect_buffer_t gridconnect_buffer;

    // ************************************************************************
    // Upper case Hex letters
    // ************************************************************************
    const char *gridconnect_pip = ":X19828BC7N06EB;";

    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[4], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[5], &gridconnect_buffer));

    for (int i = 0; i < strlen(gridconnect_pip) - 1; i++)
    {

        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[i], &gridconnect_buffer));
    }

    EXPECT_TRUE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[strlen(gridconnect_pip) - 1], &gridconnect_buffer));
    EXPECT_STREQ(":X19828BC7N06EB;", (char *)&gridconnect_buffer);

    // ************************************************************************
    // Lower case Hex letters
    // ************************************************************************
    const char *gridconnect_identify_producer = ":x19914bc7n06eb;";

    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_identify_producer[4], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_identify_producer[5], &gridconnect_buffer));

    for (int i = 0; i < strlen(gridconnect_pip) - 1; i++)
    {

        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_identify_producer[i], &gridconnect_buffer));
    }

    EXPECT_TRUE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_identify_producer[strlen(gridconnect_pip) - 1], &gridconnect_buffer));
    EXPECT_STREQ(":x19914bc7n06eb;", (char *)&gridconnect_buffer); // X and N are alw

    // ************************************************************************
    // No data
    // ************************************************************************
    const char *gridconnect_identify_events = ":X19970BC7N;";

    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_identify_events[2], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_identify_events[4], &gridconnect_buffer));

    for (int i = 0; i < strlen(gridconnect_identify_events) - 1; i++)
    {

        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_identify_events[i], &gridconnect_buffer));
    }

    EXPECT_TRUE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_identify_events[strlen(gridconnect_identify_events) - 1], &gridconnect_buffer));
    EXPECT_STREQ(":X19970BC7N;", (char *)&gridconnect_buffer);

    // ************************************************************************
    // Bad Hex Character "G" in the header
    // ************************************************************************
    const char *gridconnect_bad_hex = ":X19970GC7N;";

    for (int i = 0; i < strlen(gridconnect_bad_hex) - 1; i++)
    {

        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_bad_hex[i], &gridconnect_buffer));
    }

    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_bad_hex[strlen(gridconnect_bad_hex) - 1], &gridconnect_buffer));

    // ************************************************************************
    // Too many header characters
    // ************************************************************************
    const char *gridconnect_bad_long_header = ":X19970C75FN;";

    for (int i = 0; i < strlen(gridconnect_bad_long_header) - 1; i++)
    {

        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_bad_long_header[i], &gridconnect_buffer));
    }

    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_bad_long_header[strlen(gridconnect_bad_long_header) - 1], &gridconnect_buffer));

    // ************************************************************************
    // Too many header characters
    // ************************************************************************
    const char *gridconnect_odd_number_of_data_characters = ":X19828BC7N6EB;";

    for (int i = 0; i < strlen(gridconnect_odd_number_of_data_characters) - 1; i++)
    {

        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_odd_number_of_data_characters[i], &gridconnect_buffer));
    }

    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_odd_number_of_data_characters[strlen(gridconnect_odd_number_of_data_characters) - 1], &gridconnect_buffer));

    // ************************************************************************
    // Too few header characters
    // ************************************************************************
    const char *gridconnect_too_few_header_characters = ":X9828BC7N6EB;";

    for (int i = 0; i < strlen(gridconnect_too_few_header_characters) - 1; i++)
    {

        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_too_few_header_characters[i], &gridconnect_buffer));
    }

    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_too_few_header_characters[strlen(gridconnect_too_few_header_characters) - 1], &gridconnect_buffer));


    // ************************************************************************
    // Bad Hex Character "G" in the Data
    // ************************************************************************
    const char *gridconnect_bad_hex_in_data = ":X19970BC7N06BE0G;";

    for (int i = 0; i < strlen(gridconnect_bad_hex_in_data) - 1; i++)
    {

        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_bad_hex_in_data[i], &gridconnect_buffer));
    }

    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_bad_hex_in_data[strlen(gridconnect_bad_hex_in_data) - 1], &gridconnect_buffer));

    // ************************************************************************
    // Too many characters in the Data
    // ************************************************************************
    const char *gridconnect_too_many_data_characters = ":X19970BC7N010203040506070800;";

    for (int i = 0; i < strlen(gridconnect_too_many_data_characters) - 1; i++)
    {

        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_too_many_data_characters[i], &gridconnect_buffer));
    }

    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_too_many_data_characters[strlen(gridconnect_too_many_data_characters) - 1], &gridconnect_buffer));
}

TEST(OpenLcbGridConnect, to_can_msg)
{

    can_msg_t can_msg;

    const char *gridconnect_pip_none = ":X19828BC7N;";

    OpenLcbGridConnect_to_can_msg((gridconnect_buffer_t *)gridconnect_pip_none, &can_msg);
    EXPECT_EQ(can_msg.identifier, 0x19828BC7);
    EXPECT_EQ(can_msg.payload_count, 0);

    const char *gridconnect_pip_medium = ":X19828BC7N06EB;";

    OpenLcbGridConnect_to_can_msg((gridconnect_buffer_t *)gridconnect_pip_medium, &can_msg);
    EXPECT_EQ(can_msg.identifier, 0x19828BC7);
    EXPECT_EQ(can_msg.payload_count, 2);
    EXPECT_EQ(can_msg.payload[0], 0x06);
    EXPECT_EQ(can_msg.payload[1], 0xEB);

    const char *gridconnect_pip_full = ":X19828BC7N0102030405060708;";

    OpenLcbGridConnect_to_can_msg((gridconnect_buffer_t *)gridconnect_pip_full, &can_msg);
    EXPECT_EQ(can_msg.identifier, 0x19828BC7);
    EXPECT_EQ(can_msg.payload_count, 8);
    EXPECT_EQ(can_msg.payload[0], 0x01);
    EXPECT_EQ(can_msg.payload[1], 0x02);
    EXPECT_EQ(can_msg.payload[2], 0x03);
    EXPECT_EQ(can_msg.payload[3], 0x04);
    EXPECT_EQ(can_msg.payload[4], 0x05);
    EXPECT_EQ(can_msg.payload[5], 0x06);
    EXPECT_EQ(can_msg.payload[6], 0x07);
    EXPECT_EQ(can_msg.payload[7], 0x08);
}

TEST(OpenLcbGridConnect, from_can_msg)
{

    can_msg_t can_msg;
    gridconnect_buffer_t gridconnect_buffer;

    can_msg.identifier = 0x19828BC7;
    can_msg.payload_count = 0;

    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, &can_msg);
    
    EXPECT_STREQ((char *)&gridconnect_buffer,  ":X19828BC7N;");

    can_msg.identifier = 0x19828BC7;
    can_msg.payload_count = 2;
    can_msg.payload[0] = 0x06;
    can_msg.payload[1] = 0xEB;

    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, &can_msg);
    
    EXPECT_STREQ((char *)&gridconnect_buffer,  ":X19828BC7N06EB;");

    
    can_msg.identifier = 0x19828BC7;
    can_msg.payload_count = 8;
    can_msg.payload[0] = 0x01;
    can_msg.payload[1] = 0x02;
    can_msg.payload[2] = 0x03;
    can_msg.payload[3] = 0x04;
    can_msg.payload[4] = 0x05;
    can_msg.payload[5] = 0x06;
    can_msg.payload[6] = 0x07;
    can_msg.payload[7] = 0x08;

    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, &can_msg);
    
    EXPECT_STREQ((char *)&gridconnect_buffer,  ":X19828BC7N0102030405060708;");

}