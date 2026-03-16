#!/usr/bin/env python3
"""
update_xsd.py  —  Regenerate embedded XSD strings in the CDI and FDI validators.

When OpenLCB releases a new CDI or FDI XSD, run this script to patch the
embedded schema text in cdi_validator.js and fdi_validator.js.  The validators
use these embedded strings so the wizard works from file:// with no server.

Usage:
    python3 update_xsd.py --cdi-xsd /path/to/cdi.xsd --fdi-xsd /path/to/fdi.xsd

Either flag can be used alone if only one schema changed.

The script also copies the XSD into tools/node_wizard/schema/ so that the
Node test runner can reference them.
"""

import argparse
import os
import re
import shutil
import sys


def escape_for_js_single_quote(text):
    """Escape a string so it can be safely placed inside JS single quotes."""
    text = text.replace('\\', '\\\\')
    text = text.replace("'", "\\'")
    text = text.replace('\r', '')
    text = text.replace('\n', '\\n')
    return text


def patch_file(js_path, start_marker, end_marker, var_name, escaped_xsd):
    """Replace everything between start_marker and end_marker in js_path."""

    with open(js_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Build a regex that matches from the start marker through the end marker
    pattern = re.escape(start_marker) + r'.*?' + re.escape(end_marker)
    replacement = start_marker + "\n    var " + var_name + " = '" + escaped_xsd + "';\n    " + end_marker

    # Use lambda so re.subn does not interpret \n in escaped_xsd as newlines
    new_content, count = re.subn(pattern, lambda m: replacement, content, count=1, flags=re.DOTALL)

    if count == 0:
        print("ERROR: Could not find markers {} ... {} in {}".format(
            start_marker, end_marker, js_path), file=sys.stderr)
        return False

    with open(js_path, 'w', encoding='utf-8') as f:
        f.write(new_content)

    return True


def main():
    parser = argparse.ArgumentParser(
        description='Update embedded XSD strings in CDI/FDI validator JS files.')
    parser.add_argument('--cdi-xsd', metavar='PATH',
                        help='Path to the CDI XSD file (e.g. schema/cdi/1/4/cdi.xsd)')
    parser.add_argument('--fdi-xsd', metavar='PATH',
                        help='Path to the FDI XSD file (e.g. schema/fdi/1/1/fdi.xsd)')
    args = parser.parse_args()

    if not args.cdi_xsd and not args.fdi_xsd:
        parser.error('At least one of --cdi-xsd or --fdi-xsd is required.')

    script_dir = os.path.dirname(os.path.abspath(__file__))
    schema_dir = os.path.join(script_dir, 'schema')
    ok = True

    # --- CDI ---
    if args.cdi_xsd:
        xsd_path = os.path.abspath(args.cdi_xsd)
        if not os.path.isfile(xsd_path):
            print("ERROR: CDI XSD not found: {}".format(xsd_path), file=sys.stderr)
            ok = False
        else:
            with open(xsd_path, 'r', encoding='utf-8') as f:
                raw = f.read()

            escaped = escape_for_js_single_quote(raw)

            js_file = os.path.join(script_dir, 'cdi_editor', 'js', 'cdi_validator.js')
            if patch_file(js_file, '/* <<CDI_XSD_START>> */', '/* <<CDI_XSD_END>> */',
                          '_xsdText', escaped):
                print("Patched {}".format(js_file))
            else:
                ok = False

            # Copy XSD into schema/ for the test runner
            dest = os.path.join(schema_dir, 'cdi.xsd')
            if os.path.abspath(xsd_path) != os.path.abspath(dest):
                shutil.copy2(xsd_path, dest)
                print("Copied {} -> {}".format(xsd_path, dest))

    # --- FDI ---
    if args.fdi_xsd:
        xsd_path = os.path.abspath(args.fdi_xsd)
        if not os.path.isfile(xsd_path):
            print("ERROR: FDI XSD not found: {}".format(xsd_path), file=sys.stderr)
            ok = False
        else:
            with open(xsd_path, 'r', encoding='utf-8') as f:
                raw = f.read()

            escaped = escape_for_js_single_quote(raw)

            js_file = os.path.join(script_dir, 'fdi_editor', 'js', 'fdi_validator.js')
            if patch_file(js_file, '/* <<FDI_XSD_START>> */', '/* <<FDI_XSD_END>> */',
                          '_xsdText', escaped):
                print("Patched {}".format(js_file))
            else:
                ok = False

            dest = os.path.join(schema_dir, 'fdi.xsd')
            if os.path.abspath(xsd_path) != os.path.abspath(dest):
                shutil.copy2(xsd_path, dest)
                print("Copied {} -> {}".format(xsd_path, dest))

    if ok:
        print("\nDone.  Remember to also update the autocomplete schemas:")
        print("  python3 vendor/codemirror/generate_cdi_schema.py --xsd <path> --patch")
        print("  python3 vendor/codemirror/generate_fdi_schema.py --xsd <path> --patch")
    else:
        sys.exit(1)


if __name__ == '__main__':
    main()
