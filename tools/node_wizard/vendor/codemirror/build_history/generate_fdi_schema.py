#!/usr/bin/env python3
"""
generate_fdi_schema.py
======================
Reads the OpenLCB CDI XSD and regenerates the FDI_SCHEMA JavaScript block
used by @codemirror/lang-xml in cm-adapter.js.

Usage
-----
  # Print the generated schema to stdout:
  python3 generate_fdi_schema.py --xsd /path/to/cdi.xsd

  # Also patch cm-adapter.js in-place:
  python3 generate_fdi_schema.py --xsd /path/to/cdi.xsd --patch

How patching works
------------------
The script looks for two marker comments in cm-adapter.js:

    /* <<FDI_SCHEMA_START>> */
    ... existing schema ...
    /* <<FDI_SCHEMA_END>> */

Everything between those markers is replaced with the freshly generated
FDI_SCHEMA block.  Run this script whenever the CDI XSD is updated to keep
the autocomplete in sync with the spec.
"""

import sys
import argparse
import xml.etree.ElementTree as ET
from pathlib import Path

# ---------------------------------------------------------------------------
# Paths and constants
# ---------------------------------------------------------------------------

ADAPTER_PATH = Path(__file__).parent / 'cm-adapter.js'

MARKER_START = '/* <<FDI_SCHEMA_START>> */'
MARKER_END   = '/* <<FDI_SCHEMA_END>> */'

XS_NS = 'http://www.w3.org/2001/XMLSchema'


def xs(local):
    """Return a fully-qualified XSD tag name, e.g. xs('element')."""
    return '{' + XS_NS + '}' + local


# ---------------------------------------------------------------------------
# XSD helpers
# ---------------------------------------------------------------------------

def collect_enum_values(node):
    """Return a list of xs:enumeration values found anywhere under node."""
    values = []
    for enum in node.iter(xs('enumeration')):
        v = enum.get('value')
        if v is not None:
            values.append(v)
    return values


def collect_attributes(ct_node):
    """
    Return a list of { name, values } dicts for every xs:attribute found
    anywhere under ct_node.  Enum values are collected where present.
    """
    attrs = []
    for attr in ct_node.iter(xs('attribute')):
        name = attr.get('name')
        if not name:
            continue
        values = collect_enum_values(attr)
        attrs.append({'name': name, 'values': values})
    return attrs


def collect_children_from_container(container, named_types):
    """
    Walk an xs:sequence, xs:choice, or xs:all node and return a flat list
    of child element names (in order of first appearance).  Recurses into
    nested containers but does NOT expand named-type references — those are
    handled separately when we process each element's own spec.
    """
    children = []
    for child in container:
        tag = child.tag
        if tag == xs('element'):
            name = child.get('name') or child.get('ref', '').split(':')[-1]
            if name and name not in children:
                children.append(name)
        elif tag in (xs('sequence'), xs('choice'), xs('all')):
            for c in collect_children_from_container(child, named_types):
                if c not in children:
                    children.append(c)
    return children


def resolve_named_type(type_name, named_types):
    """
    Given a named complexType, return (children_list, attributes_list).
    """
    ct = named_types.get(type_name)
    if ct is None:
        return [], []

    children   = []
    attributes = collect_attributes(ct)

    for node in ct:
        if node.tag in (xs('sequence'), xs('choice'), xs('all')):
            children = collect_children_from_container(node, named_types)
            break

    return children, attributes


# ---------------------------------------------------------------------------
# Main XSD parser
# ---------------------------------------------------------------------------

def parse_xsd(xsd_text):
    """
    Parse the CDI XSD and return a dict:
        element_name -> { top: bool, children: [...], attributes: [...] }

    'children' is the list of valid child element names.
    'attributes' is a list of { name: str, values: [str, ...] }.
    'top' is True only for the document root element (cdi).
    """

    root         = ET.fromstring(xsd_text)
    named_types  = {}
    specs        = {}

    # ---- Collect all named complexType definitions -------------------------
    for ct in root.findall(xs('complexType')):
        name = ct.get('name')
        if name:
            named_types[name] = ct

    # ---- Helpers -----------------------------------------------------------

    visited_types = set()   # guard against self-referential types (e.g. groupType)

    def ensure(name):
        if name not in specs:
            specs[name] = {'top': False, 'children': [], 'attributes': []}
        return specs[name]

    def process_element(el):
        """Record a spec entry for a single xs:element node."""
        name = el.get('name')
        if not name:
            return
        spec = ensure(name)

        # Case 1: element references a named complexType via type="..."
        type_ref = el.get('type')
        if type_ref:
            local = type_ref.split(':')[-1]
            children, attributes = resolve_named_type(local, named_types)
            spec['children']    = children
            spec['attributes']  = attributes
            # Recurse so inline elements inside that named type are registered
            ct = named_types.get(local)
            if ct:
                recurse_into_ct(local, ct)
            return

        # Case 2: element has an inline xs:complexType
        ct = el.find(xs('complexType'))
        if ct is not None:
            spec['attributes'] = collect_attributes(ct)
            for node in ct:
                if node.tag in (xs('sequence'), xs('choice'), xs('all')):
                    spec['children'] = collect_children_from_container(node, named_types)
                    recurse_into_container(node)
                    break
                # simpleContent (e.g. linkType) — text + attrs, no children
        # else: truly empty element — no children, no attributes needed

    def recurse_into_ct(type_name, ct):
        """Walk a named complexType and process any inline xs:element nodes.
           type_name is used to detect and break self-referential cycles."""
        if type_name in visited_types:
            return
        visited_types.add(type_name)
        for node in ct:
            if node.tag in (xs('sequence'), xs('choice'), xs('all')):
                recurse_into_container(node)
        visited_types.discard(type_name)

    def recurse_into_container(container):
        """Walk a sequence/choice/all and process inline xs:element nodes."""
        for child in container:
            if child.tag == xs('element'):
                process_element(child)
            elif child.tag in (xs('sequence'), xs('choice'), xs('all')):
                recurse_into_container(child)

    # ---- Process the top-level <cdi> element ------------------------------
    for el in root.findall(xs('element')):
        name = el.get('name')
        if name == 'fdi':
            ensure('fdi')['top'] = True
            ct = el.find(xs('complexType'))
            if ct is not None:
                for node in ct:
                    if node.tag in (xs('sequence'), xs('choice'), xs('all')):
                        specs['fdi']['children'] = collect_children_from_container(node, named_types)
                        recurse_into_container(node)
                        break

    # ---- Walk all named complexTypes to catch elements defined inside them -
    for type_name, ct in named_types.items():
        recurse_into_ct(type_name, ct)

    return specs


# ---------------------------------------------------------------------------
# JavaScript output
# ---------------------------------------------------------------------------

def build_js(specs):
    """
    Convert the specs dict into the FDI_SCHEMA JavaScript block that
    @codemirror/lang-xml understands.
    """

    lines = []
    lines.append('var FDI_SCHEMA = {')
    lines.append('    elements: [')

    # cdi first, then everything else alphabetically
    names = sorted(specs.keys(), key=lambda n: (0 if n == 'fdi' else 1, n))

    for i, name in enumerate(names):

        spec    = specs[name]
        is_last = (i == len(names) - 1)

        lines.append('        {')
        lines.append("            name: '" + name + "'")

        if spec.get('top'):
            lines[-1] += ','
            lines.append('            top: true')

        if spec['attributes']:
            lines[-1] += ','
            lines.append('            attributes: [')
            for j, attr in enumerate(spec['attributes']):
                attr_comma = ',' if j < len(spec['attributes']) - 1 else ''
                if attr['values']:
                    vals = ', '.join("'" + v + "'" for v in attr['values'])
                    lines.append(
                        "                { name: '" + attr['name'] +
                        "', values: [" + vals + "] }" + attr_comma
                    )
                else:
                    lines.append(
                        "                { name: '" + attr['name'] + "' }" + attr_comma
                    )
            lines.append('            ]')

        if spec['children']:
            lines[-1] += ','
            children_str = ', '.join("'" + c + "'" for c in spec['children'])
            lines.append('            children: [' + children_str + ']')

        lines.append('        }' + ('' if is_last else ','))
        lines.append('')

    # Remove the trailing blank line before closing bracket
    if lines and lines[-1] == '':
        lines.pop()

    lines.append('    ]')
    lines.append('};')

    return '\n'.join(lines)


# ---------------------------------------------------------------------------
# Patch cm-adapter.js
# ---------------------------------------------------------------------------

def patch_adapter(js_block):
    """Replace the FDI_SCHEMA block between the markers in cm-adapter.js."""

    text = ADAPTER_PATH.read_text(encoding='utf-8')

    start_idx = text.find(MARKER_START)
    end_idx   = text.find(MARKER_END)

    if start_idx == -1 or end_idx == -1:
        print('ERROR: patch markers not found in ' + str(ADAPTER_PATH), file=sys.stderr)
        print('Add these comments around the FDI_SCHEMA block:', file=sys.stderr)
        print('  ' + MARKER_START, file=sys.stderr)
        print('  ' + MARKER_END,   file=sys.stderr)
        sys.exit(1)

    end_idx += len(MARKER_END)

    patched = (
        text[:start_idx] +
        MARKER_START + '\n' +
        js_block + '\n' +
        MARKER_END +
        text[end_idx:]
    )

    ADAPTER_PATH.write_text(patched, encoding='utf-8')
    print('Patched ' + str(ADAPTER_PATH), file=sys.stderr)


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main():

    parser = argparse.ArgumentParser(
        description='Generate FDI_SCHEMA for cm-adapter.js from the OpenLCB CDI XSD'
    )
    parser.add_argument(
        '--xsd',
        required=True,
        help='Path to the FDI XSD file  (e.g. ~/Documents/OpenLcb\ Documents/schema/fdi/1/1/fdi.xsd)'
    )
    parser.add_argument(
        '--patch',
        action='store_true',
        help='Patch cm-adapter.js in-place between the <<FDI_SCHEMA_START>> markers'
    )
    args = parser.parse_args()

    xsd_path = Path(args.xsd).expanduser()
    if not xsd_path.exists():
        print('ERROR: XSD file not found: ' + str(xsd_path), file=sys.stderr)
        sys.exit(1)

    xsd_text = xsd_path.read_text(encoding='utf-8')
    specs    = parse_xsd(xsd_text)
    js_block = build_js(specs)

    if args.patch:
        patch_adapter(js_block)
    else:
        print(js_block)


if __name__ == '__main__':
    main()
