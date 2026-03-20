#!/bin/bash

echo "Updating: arduino/esp32/BasicNode"
cp ../../src/openlcb/*.c ../../src/applications/arduino/esp32/BasicNode/src/openlcb_c_lib/openlcb/
cp ../../src/openlcb/*.h ../../src/applications/arduino/esp32/BasicNode/src/openlcb_c_lib/openlcb/
cp ../../src/drivers/canbus/*.c ../../src/applications/arduino/esp32/BasicNode/src/openlcb_c_lib/drivers/canbus/
cp ../../src/drivers/canbus/*.h ../../src/applications/arduino/esp32/BasicNode/src/openlcb_c_lib/drivers/canbus/
cp ../../src/utilities/*.c ../../src/applications/arduino/esp32/BasicNode/src/openlcb_c_lib/utilities/
cp ../../src/utilities/*.h ../../src/applications/arduino/esp32/BasicNode/src/openlcb_c_lib/utilities/

echo "Updating: arduino/rpi_pico/BasicNode"
cp ../../src/openlcb/*.c ../../src/applications/arduino/rpi_pico/BasicNode/src/openlcb_c_lib/openlcb/
cp ../../src/openlcb/*.h ../../src/applications/arduino/rpi_pico/BasicNode/src/openlcb_c_lib/openlcb/
cp ../../src/drivers/canbus/*.c ../../src/applications/arduino/rpi_pico/BasicNode/src/openlcb_c_lib/drivers/canbus/
cp ../../src/drivers/canbus/*.h ../../src/applications/arduino/rpi_pico/BasicNode/src/openlcb_c_lib/drivers/canbus/
cp ../../src/utilities/*.c ../../src/applications/arduino/rpi_pico/BasicNode/src/openlcb_c_lib/utilities/
cp ../../src/utilities/*.h ../../src/applications/arduino/rpi_pico/BasicNode/src/openlcb_c_lib/utilities/

echo "Updating: platformio/esp32/BasicNode"
cp ../../src/openlcb/*.c ../../src/applications/platformio/esp32/BasicNode/src/openlcb_c_lib/openlcb/
cp ../../src/openlcb/*.h ../../src/applications/platformio/esp32/BasicNode/src/openlcb_c_lib/openlcb/
cp ../../src/drivers/canbus/*.c ../../src/applications/platformio/esp32/BasicNode/src/openlcb_c_lib/drivers/canbus/
cp ../../src/drivers/canbus/*.h ../../src/applications/platformio/esp32/BasicNode/src/openlcb_c_lib/drivers/canbus/
cp ../../src/utilities/*.c ../../src/applications/platformio/esp32/BasicNode/src/openlcb_c_lib/utilities/
cp ../../src/utilities/*.h ../../src/applications/platformio/esp32/BasicNode/src/openlcb_c_lib/utilities/

echo "Updating: platformio/esp32/BasicNode_WiFi"
cp ../../src/openlcb/*.c ../../src/applications/platformio/esp32/BasicNode_WiFi/src/openlcb_c_lib/openlcb/
cp ../../src/openlcb/*.h ../../src/applications/platformio/esp32/BasicNode_WiFi/src/openlcb_c_lib/openlcb/
cp ../../src/drivers/canbus/*.c ../../src/applications/platformio/esp32/BasicNode_WiFi/src/openlcb_c_lib/drivers/canbus/
cp ../../src/drivers/canbus/*.h ../../src/applications/platformio/esp32/BasicNode_WiFi/src/openlcb_c_lib/drivers/canbus/
cp ../../src/utilities/*.c ../../src/applications/platformio/esp32/BasicNode_WiFi/src/openlcb_c_lib/utilities/
cp ../../src/utilities/*.h ../../src/applications/platformio/esp32/BasicNode_WiFi/src/openlcb_c_lib/utilities/

echo "Updating: platformio/osx/BasicNode"
cp ../../src/openlcb/*.c ../../src/applications/platformio/osx/BasicNode/src/openlcb_c_lib/openlcb/
cp ../../src/openlcb/*.h ../../src/applications/platformio/osx/BasicNode/src/openlcb_c_lib/openlcb/
cp ../../src/drivers/canbus/*.c ../../src/applications/platformio/osx/BasicNode/src/openlcb_c_lib/drivers/canbus/
cp ../../src/drivers/canbus/*.h ../../src/applications/platformio/osx/BasicNode/src/openlcb_c_lib/drivers/canbus/
cp ../../src/utilities/*.c ../../src/applications/platformio/osx/BasicNode/src/openlcb_c_lib/utilities/
cp ../../src/utilities/*.h ../../src/applications/platformio/osx/BasicNode/src/openlcb_c_lib/utilities/
cp ../../src/utilities_pc/*.c ../../src/applications/platformio/osx/BasicNode/src/openlcb_c_lib/utilities/
cp ../../src/utilities_pc/*.h ../../src/applications/platformio/osx/BasicNode/src/openlcb_c_lib/utilities/

echo "Updating: xcode/BasicNode"
cp ../../src/openlcb/*.c ../../src/applications/xcode/BasicNode/openlcb_c_lib/openlcb/
cp ../../src/openlcb/*.h ../../src/applications/xcode/BasicNode/openlcb_c_lib/openlcb/
cp ../../src/drivers/canbus/*.c ../../src/applications/xcode/BasicNode/openlcb_c_lib/drivers/canbus/
cp ../../src/drivers/canbus/*.h ../../src/applications/xcode/BasicNode/openlcb_c_lib/drivers/canbus/
cp ../../src/utilities/*.c ../../src/applications/xcode/BasicNode/openlcb_c_lib/utilities/
cp ../../src/utilities/*.h ../../src/applications/xcode/BasicNode/openlcb_c_lib/utilities/
cp ../../src/utilities_pc/*.c ../../src/applications/xcode/BasicNode/openlcb_c_lib/utilities/
cp ../../src/utilities_pc/*.h ../../src/applications/xcode/BasicNode/openlcb_c_lib/utilities/

echo "Updating: ti_thiea/mspm03507_launchpad/BasicNode"
cp ../../src/openlcb/*.c ../../src/applications/ti_thiea/mspm03507_launchpad/BasicNode/openlcb_c_lib/openlcb/
cp ../../src/openlcb/*.h ../../src/applications/ti_thiea/mspm03507_launchpad/BasicNode/openlcb_c_lib/openlcb/
cp ../../src/drivers/canbus/*.c ../../src/applications/ti_thiea/mspm03507_launchpad/BasicNode/openlcb_c_lib/drivers/canbus/
cp ../../src/drivers/canbus/*.h ../../src/applications/ti_thiea/mspm03507_launchpad/BasicNode/openlcb_c_lib/drivers/canbus/
cp ../../src/utilities/*.c ../../src/applications/ti_thiea/mspm03507_launchpad/BasicNode/openlcb_c_lib/utilities/
cp ../../src/utilities/*.h ../../src/applications/ti_thiea/mspm03507_launchpad/BasicNode/openlcb_c_lib/utilities/

echo "Updating: dspic/BasicNode.X"
cp ../../src/openlcb/*.c ../../src/applications/dspic/BasicNode.X/src/openlcb_c_lib/openlcb/
cp ../../src/openlcb/*.h ../../src/applications/dspic/BasicNode.X/src/openlcb_c_lib/openlcb/
cp ../../src/drivers/canbus/*.c ../../src/applications/dspic/BasicNode.X/src/openlcb_c_lib/drivers/canbus/
cp ../../src/drivers/canbus/*.h ../../src/applications/dspic/BasicNode.X/src/openlcb_c_lib/drivers/canbus/
cp ../../src/utilities/*.c ../../src/applications/dspic/BasicNode.X/src/openlcb_c_lib/utilities/
cp ../../src/utilities/*.h ../../src/applications/dspic/BasicNode.X/src/openlcb_c_lib/utilities/

echo "Updating: stm32_cubeide/stm32f407_discovery/BasicNode"
cp ../../src/openlcb/*.c ../../src/applications/stm32_cubeide/stm32f407_discovery/BasicNode/Core/Src/openlcb_c_lib/openlcb/
cp ../../src/openlcb/*.h ../../src/applications/stm32_cubeide/stm32f407_discovery/BasicNode/Core/Src/openlcb_c_lib/openlcb/
cp ../../src/drivers/canbus/*.c ../../src/applications/stm32_cubeide/stm32f407_discovery/BasicNode/Core/Src/openlcb_c_lib/drivers/canbus/
cp ../../src/drivers/canbus/*.h ../../src/applications/stm32_cubeide/stm32f407_discovery/BasicNode/Core/Src/openlcb_c_lib/drivers/canbus/
cp ../../src/utilities/*.c ../../src/applications/stm32_cubeide/stm32f407_discovery/BasicNode/Core/Src/openlcb_c_lib/utilities/
cp ../../src/utilities/*.h ../../src/applications/stm32_cubeide/stm32f407_discovery/BasicNode/Core/Src/openlcb_c_lib/utilities/

echo "Updating: stm32_cubeide/stm32f407_discovery/BasicNodeBootloader"
cp ../../src/openlcb/*.c ../../src/applications/stm32_cubeide/stm32f407_discovery/BasicNodeBootloader/Core/Src/openlcb_c_lib/openlcb/
cp ../../src/openlcb/*.h ../../src/applications/stm32_cubeide/stm32f407_discovery/BasicNodeBootloader/Core/Src/openlcb_c_lib/openlcb/
cp ../../src/drivers/canbus/*.c ../../src/applications/stm32_cubeide/stm32f407_discovery/BasicNodeBootloader/Core/Src/openlcb_c_lib/drivers/canbus/
cp ../../src/drivers/canbus/*.h ../../src/applications/stm32_cubeide/stm32f407_discovery/BasicNodeBootloader/Core/Src/openlcb_c_lib/drivers/canbus/
cp ../../src/utilities/*.c ../../src/applications/stm32_cubeide/stm32f407_discovery/BasicNodeBootloader/Core/Src/openlcb_c_lib/utilities/
cp ../../src/utilities/*.h ../../src/applications/stm32_cubeide/stm32f407_discovery/BasicNodeBootloader/Core/Src/openlcb_c_lib/utilities/

echo "Updating: NmraLccProjects/RPiPico"
cp ../../src/openlcb/*.c ../../../NmraLccProjects/RPiPico/src/openlcb/
cp ../../src/openlcb/*.h ../../../NmraLccProjects/RPiPico/src/openlcb/
cp ../../src/drivers/canbus/*.c ../../../NmraLccProjects/RPiPico/src/drivers/canbus/
cp ../../src/drivers/canbus/*.h ../../../NmraLccProjects/RPiPico/src/drivers/canbus/
cp ../../src/utilities/*.c ../../../NmraLccProjects/RPiPico/src/utilities/
cp ../../src/utilities/*.h ../../../NmraLccProjects/RPiPico/src/utilities/

echo "Updating: NmraLccProjects/RPiPico_Train"
cp ../../src/openlcb/*.c ../../../NmraLccProjects/RPiPico_Train/src/openlcb/
cp ../../src/openlcb/*.h ../../../NmraLccProjects/RPiPico_Train/src/openlcb/
cp ../../src/drivers/canbus/*.c ../../../NmraLccProjects/RPiPico_Train/src/drivers/canbus/
cp ../../src/drivers/canbus/*.h ../../../NmraLccProjects/RPiPico_Train/src/drivers/canbus/
cp ../../src/utilities/*.c ../../../NmraLccProjects/RPiPico_Train/src/utilities/
cp ../../src/utilities/*.h ../../../NmraLccProjects/RPiPico_Train/src/utilities/

echo "Updating: LCC Projects/TurnoutBoss2_Firmware/TurnoutBossCommon"
cp ../../src/openlcb/*.c "../../../LCC Projects/TurnoutBoss2_Firmware/TurnoutBossCommon/openlcb/"
cp ../../src/openlcb/*.h "../../../LCC Projects/TurnoutBoss2_Firmware/TurnoutBossCommon/openlcb/"
cp ../../src/drivers/canbus/*.c "../../../LCC Projects/TurnoutBoss2_Firmware/TurnoutBossCommon/drivers/canbus/"
cp ../../src/drivers/canbus/*.h "../../../LCC Projects/TurnoutBoss2_Firmware/TurnoutBossCommon/drivers/canbus/"
cp ../../src/utilities/*.c "../../../LCC Projects/TurnoutBoss2_Firmware/TurnoutBossCommon/utilities/"
cp ../../src/utilities/*.h "../../../LCC Projects/TurnoutBoss2_Firmware/TurnoutBossCommon/utilities/"

echo "Done."
