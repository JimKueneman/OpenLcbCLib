# XmlToArray.py
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
#   python XmlToArray.py [options] [input.xml]
#
# If no input file is given, defaults to "sample.xml".
# Output is written to input.xml.c (or use -o to override).

import sys
import os
import re
import datetime
import xml.dom.minidom


def print_help():

    print("XmlToArray - Convert XML/CDI/FDI to a C hex byte array")
    print("")
    print("Usage: python XmlToArray.py [options] [input.xml]")
    print("")
    print("Options:")
    print("  -h, --help           Show this help message")
    print("  -f, --format         Pretty-print the XML (one element per line) before")
    print("                       converting. Useful for single-line XML sources.")
    print("  -w, --whitespace     Preserve whitespace (indentation, newlines)")
    print("  -n, --name NAME      C variable name for the array (default: _cdi_data)")
    print("  -t, --type TYPE      Shorthand: 'cdi' sets name to _cdi_data,")
    print("                       'fdi' sets name to _fdi_data")
    print("  -o, --output PATH    Output base path (generates PATH.c and optionally PATH.h)")
    print("  --header             Also generate a .h file with extern declaration")
    print("  --author NAME        Author name for the file header comment")
    print("  --company NAME       Company/copyright holder for the file header comment")
    print("  --license PATH       Path to a custom license text file to embed in the")
    print("                       header instead of the default BSD 2-clause license")
    print("")
    print("Default behavior:")
    print("  - XML comments are always stripped")
    print("  - Whitespace (indentation, newlines, blank lines) is stripped")
    print("  - Output uses uppercase hex with null terminator")
    print("  - Each input line becomes one row with // comment showing original text")
    print("  - Array is declared 'static const uint8_t' (or 'const' with --header)")
    print("")
    print("Combining -f and -w preserves the pretty-printed indentation and newlines")
    print("in the byte array.")
    print("")
    print("Examples:")
    print("  python XmlToArray.py my_cdi.xml")
    print("      -> my_cdi.xml.c  (static const uint8_t _cdi_data[])")
    print("")
    print("  python XmlToArray.py -t cdi --header -o my_cdi my_cdi.xml")
    print("      -> my_cdi.c + my_cdi.h  (extern const uint8_t _cdi_data[])")
    print("")
    print("  python XmlToArray.py -t fdi --header -o train_fdi train_fdi.xml")
    print("      -> train_fdi.c + train_fdi.h  (extern const uint8_t _fdi_data[])")
    print("")
    print("  python XmlToArray.py --author 'Jane Smith' --company 'Acme Rail' \\")
    print("      -t cdi --header -o my_cdi my_cdi.xml")
    print("      -> my_cdi.c + my_cdi.h with BSD license header")
    print("")
    print("If no input file is given, defaults to 'sample.xml'.")


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


def _make_header_guard(basename):
    """Convert a filename base into a header guard: my_cdi -> __MY_CDI_H__"""

    return '__' + re.sub(r'[^A-Za-z0-9]', '_', basename).upper() + '_H__'


def _file_header(filename, brief, author, company, source_xml, license_text):
    """Generate a license + doxygen file header block.

    If license_text is provided it replaces the default BSD 2-clause body.
    Each line of the custom text is wrapped with ' * ' to fit inside the
    comment block.
    """

    year = datetime.date.today().year
    date_str = datetime.date.today().strftime('%d %b %Y')
    holder = company if company else author if author else None

    lines = []
    lines.append('/** \\copyright')

    if license_text:

        # Wrap each line of the custom license inside the comment block
        for lline in license_text.splitlines():
            if lline.strip():
                lines.append(' * ' + lline)
            else:
                lines.append(' *')

    else:

        # Default BSD 2-clause license
        if holder:
            lines.append(' * Copyright (c) ' + str(year) + ', ' + holder)
        else:
            lines.append(' * Copyright (c) ' + str(year))

        lines.append(' * All rights reserved.')
        lines.append(' *')
        lines.append(' * Redistribution and use in source and binary forms, with or without')
        lines.append(' * modification, are permitted provided that the following conditions are met:')
        lines.append(' *')
        lines.append(' *  - Redistributions of source code must retain the above copyright notice,')
        lines.append(' *    this list of conditions and the following disclaimer.')
        lines.append(' *')
        lines.append(' *  - Redistributions in binary form must reproduce the above copyright notice,')
        lines.append(' *    this list of conditions and the following disclaimer in the documentation')
        lines.append(' *    and/or other materials provided with the distribution.')
        lines.append(' *')
        lines.append(' * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"')
        lines.append(' * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE')
        lines.append(' * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE')
        lines.append(' * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE')
        lines.append(' * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR')
        lines.append(' * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF')
        lines.append(' * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS')
        lines.append(' * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN')
        lines.append(' * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)')
        lines.append(' * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE')
        lines.append(' * POSSIBILITY OF SUCH DAMAGE.')

    lines.append(' *')
    lines.append(' * @file ' + filename)
    lines.append(' * @brief ' + brief)

    if source_xml:
        lines.append(' *')
        lines.append(' * @details Auto-generated from ' + source_xml + ' by XmlToArray.py.')
        lines.append(' *          Do not edit by hand — regenerate from the source XML.')

    lines.append(' *')

    if author:
        lines.append(' * @author ' + author)

    lines.append(' * @date ' + date_str)
    lines.append(' */')
    lines.append('')

    return '\n'.join(lines)


def convert(infilename, keep_whitespace, format_first, array_name, out_base,
            gen_header, author, company, license_text):

    # Determine output filenames
    if out_base:
        out_c = out_base + '.c'
        out_h = out_base + '.h'
    else:
        out_c = infilename + '.c'
        out_h = infilename + '.h'

    # static when standalone fragment, extern linkage when generating a .h
    storage = 'const' if gen_header else 'static const'

    # Determine type label for the brief
    if '_fdi_' in array_name or array_name.endswith('_fdi'):
        type_label = 'FDI'
    else:
        type_label = 'CDI'

    source_xml = os.path.basename(infilename)

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

    with open(out_c, 'w') as outfile:

        # File header
        if gen_header:
            c_basename = os.path.basename(out_c)
            brief = type_label + ' XML byte array for OpenLCB node parameters.'
            outfile.write(_file_header(c_basename, brief, author, company, source_xml, license_text))
            outfile.write('\n')
            h_basename = os.path.basename(out_h)
            outfile.write('#include "' + h_basename + '"\n')
            outfile.write('\n\n')

        outfile.write(storage + ' uint8_t ' + array_name + '[] = {\n\n')

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

            outfile.write("    " + ", ".join(hex_values))
            outfile.write(",  // " + line.strip() + "\n")

        # Null terminator
        outfile.write("    0x00\n")
        charcount += 1

        outfile.write("\n};\n")

    print("Transformed " + str(charcount) + " bytes (including null terminator)")
    print("Output written to " + out_c)

    # Generate matching .h file
    if gen_header:

        base = os.path.basename(out_base) if out_base else infilename
        guard = _make_header_guard(base)
        h_basename = os.path.basename(out_h)
        brief = type_label + ' XML byte array declaration for OpenLCB node parameters.'

        with open(out_h, 'w') as hfile:

            hfile.write(_file_header(h_basename, brief, author, company, source_xml, license_text))
            hfile.write('\n')
            hfile.write('#ifndef ' + guard + '\n')
            hfile.write('#define ' + guard + '\n')
            hfile.write('\n')
            hfile.write('#include <stdint.h>\n')
            hfile.write('\n')
            hfile.write('#ifdef __cplusplus\n')
            hfile.write('extern "C" {\n')
            hfile.write('#endif\n')
            hfile.write('\n')
            hfile.write('extern const uint8_t ' + array_name + '[];\n')
            hfile.write('\n')
            hfile.write('#ifdef __cplusplus\n')
            hfile.write('}\n')
            hfile.write('#endif\n')
            hfile.write('\n')
            hfile.write('#endif  /* ' + guard + ' */\n')

        print("Header written to " + out_h)


# Parse arguments
keep_whitespace = False
format_first = False
gen_header = False
array_name = '_cdi_data'
out_base = None
author = None
company = None
license_text = None
infilename = 'sample.xml'

args = list(sys.argv[1:])

if '-h' in args or '--help' in args:
    print_help()
    sys.exit(0)

# Extract flag-with-value options first
i = 0
positional = []

while i < len(args):

    if args[i] in ('-n', '--name') and i + 1 < len(args):
        array_name = args[i + 1]
        i += 2

    elif args[i] in ('-t', '--type') and i + 1 < len(args):
        t = args[i + 1].lower()
        if t == 'cdi':
            array_name = '_cdi_data'
        elif t == 'fdi':
            array_name = '_fdi_data'
        else:
            print("Error: -t must be 'cdi' or 'fdi'")
            sys.exit(1)
        i += 2

    elif args[i] in ('-o', '--output') and i + 1 < len(args):
        out_base = args[i + 1]
        i += 2

    elif args[i] == '--author' and i + 1 < len(args):
        author = args[i + 1]
        i += 2

    elif args[i] == '--company' and i + 1 < len(args):
        company = args[i + 1]
        i += 2

    elif args[i] == '--license' and i + 1 < len(args):
        with open(args[i + 1], 'r') as lf:
            license_text = lf.read().rstrip('\n')
        i += 2

    elif args[i] in ('-w', '--whitespace'):
        keep_whitespace = True
        i += 1

    elif args[i] in ('-f', '--format'):
        format_first = True
        i += 1

    elif args[i] == '--header':
        gen_header = True
        i += 1

    else:
        positional.append(args[i])
        i += 1

if len(positional) > 0:
    infilename = positional[0]

convert(infilename, keep_whitespace, format_first, array_name, out_base,
        gen_header, author, company, license_text)
