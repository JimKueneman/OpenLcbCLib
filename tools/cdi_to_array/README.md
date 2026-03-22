CdiToArray.py:

    Converts an XML file into a C-style hex byte array for embedding in the
    node parameters structure. XML comments are always stripped. By default
    whitespace (indentation, newlines, blank lines) is also stripped to
    minimize the byte array size. A null terminator (0x00) is always appended.

    Usage:  python CdiToArray.py [options] [input.xml]

    Options:
      -h, --help        Show help message
      -w, --whitespace  Preserve whitespace (indentation, newlines)

    If no input file is given, defaults to "sample.xml".
    The output file is named <input>.c and contains hex values with
    // comments showing the original text for each line.
