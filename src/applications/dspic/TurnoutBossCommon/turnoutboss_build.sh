
echo "Converting the hex file to a dshex file..."
pwd
../TurnoutBossCommon/Hex2dsHex/hex2dshex -f ../TurnoutBOSS.X/dist/default/production/TurnoutBOSS.X.production.hex -m 0xB000 -x 0x55000 -e 1024 -c 0x54800 -k MustangpeakEngineeringTurnoutBoss2.0


echo "Generating the Merged Hex File (currently not useful as it does not have the CheckSum embedded in it)"

echo "Generating a Big Endian Merged Version (probably the correct way to do it)"

"/Applications/microchip/mplabx/v6.20/mplab_platform/bin/hexmate" r0xB000-0x55800,"../TurnoutBOSS.X/dist/default/production/TurnoutBOSS.X.production.hex" r0x0000-0xB000,"../TurnoutBossBootloader.X/dist/default/production/TurnoutBossBootloader.X.production.hex" -fill=w1:0xAA@0xB000:0x53FFC -ck=0xB000-0x53FFC@0x54800w3g3 -addressing=2 -o../turnoutboss_merged_big_endian.hex 


echo "Generating a Little Endian Merged Version"

"/Applications/microchip/mplabx/v6.20/mplab_platform/bin/hexmate" r0xB000-0x55800,"../TurnoutBOSS.X/dist/default/production/TurnoutBOSS.X.production.hex" r0x0000-0xB000,"../TurnoutBossBootloader.X/dist/default/production/TurnoutBossBootloader.X.production.hex" -fill=w1:0xAA@0xB000:0x53FFC -ck=0xB000-0x53FFC@0x54800w-3g3 -addressing=2 -o../turnoutboss_merged_little_endian.hex 



