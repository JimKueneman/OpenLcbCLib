#include "test/main_Test.hxx"

#include "openlcb_gridconnect.h"

#include "openlcb_types.h"

TEST(OpenLcbGridConnect, copy_out_gridconnect_when_done)
{

    gridconnect_buffer_t gridconnect_buffer;

    const char *gridconnect_pip = ":X19828BC7FN06EB;";

    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[0], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[1], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[2], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[3], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[4], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[5], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[6], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[7], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[8], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[9], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[10], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[11], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[12], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[13], &gridconnect_buffer));
    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[14], &gridconnect_buffer));

    EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(0x00, &gridconnect_buffer));

    // while (!OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[i], &gridconnect_buffer))
    // {

    //     i++;
    // }
}