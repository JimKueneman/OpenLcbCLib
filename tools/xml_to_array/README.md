XmlToArray.py:

    Converts an XML file (CDI, FDI, or any XML) into a C-style hex byte
    array for embedding in OpenLCB node parameters. XML comments are always
    stripped. By default whitespace (indentation, newlines, blank lines) is
    also stripped to minimize the byte array size. A null terminator (0x00)
    is always appended.

    Usage:  python XmlToArray.py [options] [input.xml]

    Options:
      -h, --help           Show help message
      -f, --format         Pretty-print the XML before converting
      -w, --whitespace     Preserve whitespace (indentation, newlines)
      -n, --name NAME      C variable name for the array (default: _cdi_data)
      -t, --type TYPE      Shorthand: 'cdi' or 'fdi' (sets variable name)
      -o, --output PATH    Output base path (generates PATH.c and PATH.h)
      --header             Also generate a .h file with extern declaration
      --author NAME        Author name for the file header comment
      --company NAME       Company/copyright holder for the file header
      --license PATH       Path to custom license text file for the header

    If no input file is given, defaults to "sample.xml".

    Default mode (no --header):
      Outputs <input>.c containing a static const uint8_t array fragment
      suitable for pasting into an existing openlcb_user_config.c file.

    Standalone mode (--header):
      Generates a .c/.h pair with BSD 2-clause license header (or custom
      license via --license), extern linkage, C++ guards, and doxygen tags.
      The .c file #includes the .h and the array can be referenced from
      openlcb_user_config.c via #include.

    Examples:
      python XmlToArray.py my_cdi.xml
          -> my_cdi.xml.c  (static const uint8_t _cdi_data[])

      python XmlToArray.py -t cdi --header -o my_cdi my_cdi.xml
          -> my_cdi.c + my_cdi.h  (extern const uint8_t _cdi_data[])

      python XmlToArray.py -t fdi --header -o train_fdi train_fdi.xml
          -> train_fdi.c + train_fdi.h  (extern const uint8_t _fdi_data[])

      python XmlToArray.py --author "Jane Smith" --company "Acme Rail" \
          --license my_license.txt -t cdi --header -o my_cdi my_cdi.xml
          -> my_cdi.c + my_cdi.h with custom license header
