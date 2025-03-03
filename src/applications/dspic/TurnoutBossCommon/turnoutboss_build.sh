
echo "Converting the hex file to a dshex file..."

/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/TurnoutBossCommon/Hex2dsHex/hex2dshex -f /Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/TurnoutBOSS.X/dist/default/production/TurnoutBOSS.X.production.hex -m 0xB000 -x 0x55000 -e 1024 -c 0x54800 -k MustangpeakEngineeringTurnoutBoss2.0


echo "Generating the Merged Hex File (currently not useful as it does not have the CheckSum embedded in it)"

"/Applications/microchip/mplabx/v6.20/mplab_platform/bin/hexmate" r0xB000-0x55800,"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/TurnoutBOSS.X/dist/default/production/TurnoutBOSS.X.production.hex" r0x0000-0xB000,"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/TurnoutBossBootloader.X/dist/default/production/TurnoutBossBootloader.X.production.hex" -addressing=2 -o/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/turnoutboss_merged.hex 




