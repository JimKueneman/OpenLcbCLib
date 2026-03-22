# CdiToArray.py
#
# Converts an XML file into a C-style hex byte array for embedding in
# OpenLCB node parameters.
#
# Preprocessing:
#   - XML comments (<!-- ... -->) are always stripped
#   - Use -f to pretty-print (reformat) the XML before conversion
#   - By default whitespace (indentation, newlines, blank lines) is stripped
#   - Use -w to preserve whitespace
#
# Output always includes a null terminator (0x00) and reports the byte count.
#
# Usage:
#   python CdiToArray.py [-f] [-w] [-h] [input.xml]
#
# If no input file is given, defaults to "sample.xml".
# Output is written to input.xml.c

import sys
import re
import xml.dom.minidom


def print_help():

    print("CdiToArray - Convert XML/CDI to a C hex byte array")
    print("")
    print("Usage: python CdiToArray.py [options] [input.xml]")
    print("")
    print("Options:")
    print("  -h, --help        Show this help message")
    print("  -f, --format      Pretty-print the XML (one element per line) before")
    print("                    converting. Useful for single-line XML sources.")
    print("  -w, --whitespace  Preserve whitespace (indentation, newlines)")
    print("")
    print("Default behavior:")
    print("  - XML comments are always stripped")
    print("  - Whitespace (indentation, newlines, blank lines) is stripped")
    print("  - Output uses uppercase hex with null terminator")
    print("  - Each input line becomes one row with // comment showing original text")
    print("")
    print("Combining -f and -w preserves the pretty-printed indentation and newlines")
    print("in the byte array.")
    print("")
    print("If no input file is given, defaults to 'sample.xml'.")
    print("Output is written to <input>.c")


def strip_comments(text):
    """Remove all XML comments (<!-- ... -->) including multi-line."""

    return re.sub(r'<!--.*?-->', '', text, flags=re.DOTALL)


def format_xml(text):
    """Pretty-print XML with one element per line, 2-space indent."""

    dom = xml.dom.minidom.parseString(text)
    pretty = dom.toprettyxml(indent="  ", encoding=None)

    # toprettyxml generates <?xml version="1.0" ?> — normalize it
    pretty = pretty.replace('<?xml version="1.0" ?>', '<?xml version="1.0"?>')

    # Remove blank lines that toprettyxml sometimes inserts
    lines = [line for line in pretty.split('\n') if line.strip()]

    return '\n'.join(lines)


def convert(infilename, keep_whitespace, format_first):

    outfilename = infilename + ".c"

    with open(infilename, 'r') as f:
        raw = f.read()

    # Normalize line endings (strip \r from Windows CRLF)
    raw = raw.replace('\r\n', '\n').replace('\r', '\n')

    # Always strip comments
    cleaned = strip_comments(raw)

    # Optionally pretty-print the XML
    if format_first:
        cleaned = format_xml(cleaned)

    # Split into lines for processing
    lines = cleaned.split('\n')

    charcount = 0

    with open(outfilename, 'w') as outfile:

        for line in lines:

            if not keep_whitespace:
                line = line.strip()

            # Skip blank lines (and whitespace-only lines after comment stripping)
            if len(line.strip()) == 0:
                continue

            # Write hex bytes (UTF-8 encoded)
            hex_values = []
            for byte in line.encode('utf-8'):
                hex_values.append("0x{:02X}".format(byte))
                charcount += 1

            if keep_whitespace:
                # Add newline byte
                hex_values.append("0x0A")
                charcount += 1

            outfile.write(", ".join(hex_values))
            outfile.write(",  // " + line.strip() + "\n")

        # Null terminator
        outfile.write("0x00\n")
        charcount += 1

    print("Transformed " + str(charcount) + " bytes (including null terminator)")
    print("Output written to " + outfilename)


# Parse arguments
keep_whitespace = False
format_first = False
infilename = "sample.xml"

args = [a for a in sys.argv[1:]]

if '-h' in args or '--help' in args:
    print_help()
    sys.exit(0)

if '-w' in args or '--whitespace' in args:
    keep_whitespace = True
    args = [a for a in args if a not in ('-w', '--whitespace')]

if '-f' in args or '--format' in args:
    format_first = True
    args = [a for a in args if a not in ('-f', '--format')]

# Remaining arg is the filename
if len(args) > 0:
    infilename = args[0]

convert(infilename, keep_whitespace, format_first)
