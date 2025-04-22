
# echo "Converting the hex file to a dshex file..."

#../TurnoutBossCommon/Hex2dsHex/hex2dshex -f ../TurnoutBOSS.X/dist/default/production/TurnoutBOSS.X.production.hex -m 0xB000 -x 0x55000 -e 1024 -c 0x54800 -k MustangpeakEngineeringTurnoutBossRevC


echo "Generating the Merged Hex File"

"/Applications/microchip/mplabx/v6.20/mplab_platform/bin/hexmate" r0xB000-0x55800,"../TurnoutBOSS.X/dist/default/production/TurnoutBOSS.X.production.hex" r0x0000-0xB000,"../TurnoutBossBootloader.X/dist/default/production/TurnoutBossBootloader.X.production.hex" -addressing=2 -o../turnoutboss_merged.hex 

echo "Generating the hex file with checksum information"

# first -fill inserts the start address
# second -fill inserts the end address
# third -fill put 0 in otherwise not loaded memory
# the -ck inserts a one-byte checksum

# "/Applications/microchip/mplabx/v6.20/mplab_platform/bin/hexmate" ../turnoutboss_merged.hex -o../turnoutboss_merged_summed.hex -fill=w1:0x00,0x60,0x00,0x00@0xa9000:0xa9002 -fill=w1:0xFF,0x47,0x05,0x00@0xa9004:0xa9007 -fill=00@0x6000:0x547ff -ck=0C000-0a8fff@a9008w1g1

# "/Applications/microchip/mplabx/v6.20/mplab_platform/bin/hexmate" ../turnoutboss_merged.hex -o../turnoutboss_merged_summed.hex -addressing=2 -fill=w1:0x00,0x60,0x00,0x00@0x54800:0x54801 -fill=w1:0xFF,0x47,0x05,0x00@0x54802:0x54803 -fill=00@0x6000:0x547ff -ck=06000-0547ff@54804w1g1

"/Applications/microchip/mplabx/v6.20/mplab_platform/bin/hexmate" ../turnoutboss_merged.hex -o../turnoutboss_merged_summed.hex -addressing=2 -fill=w1:0x00,0xB0,0x00,0x00@0x54800:0x54801 -fill=w1:0xFF,0x3F,0x05,0x00@0x54802:0x54803 -fill=00@0xB000:0x53FFF -ck=0B000-0538ff@54804w1g1

echo "Converting the hex file to a dshex file..."

# Bob use this for the Rev B boards you have:
#../TurnoutBossCommon/Hex2dsHex/hex2dshex -f ../turnoutboss_merged_summed.hex -m 0xB000 -x 0x55000 -e 1024 -c 0x54800 -k MustangpeakEngineeringTurnoutBossRevB

../TurnoutBossCommon/Hex2dsHex/hex2dshex -f ../turnoutboss_merged_summed.hex -m 0xB000 -x 0x55000 -e 1024 -c 0x54800 -k MustangpeakEngineeringTurnoutBossRevC

echo "Deleting the Merged file"

rm ../turnoutboss_merged.hex