# read a CDI file and write out a file with array contents

import sys

if len(sys.argv) > 1:
    first_param = sys.argv[1]
    #print(f"The first parameter passed is: {first_param}")
    infilename = first_param
else:
    print("No parameters were passed to the script.")
    infilename = "sample.xml"

#print(f"All arguments: {sys.argv}")

outfilename = infilename+".c"

infile = open(infilename)
outfile = open(outfilename, "w")
charcount = 0

for line in infile :
    for char in line:
        charcount = charcount+1
        outfile.write(hex(ord(char))+", ")
    outfile.write("  // "+line)
infile.close()
outfile.close()
print("transformed "+str(charcount)+" characters")

