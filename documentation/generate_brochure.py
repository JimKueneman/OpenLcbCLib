#!/usr/bin/env python3
"""Generate the OpenLcbCLib Brochure PDF with current project data."""

from reportlab.lib.pagesizes import letter
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.units import inch
from reportlab.lib.colors import HexColor, black, white
from reportlab.lib.enums import TA_LEFT, TA_CENTER, TA_RIGHT
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle,
    PageBreak, KeepTogether
)

OUTPUT = "OpenLcbCLib_Brochure.pdf"

# Colors
DARK_BLUE = HexColor("#1a3a5c")
MED_BLUE = HexColor("#2b5f8a")
LIGHT_BLUE = HexColor("#e8f0f8")
ACCENT = HexColor("#3a7ebf")
DARK_GRAY = HexColor("#333333")
MED_GRAY = HexColor("#666666")
LIGHT_GRAY = HexColor("#f0f0f0")
TABLE_HEADER = HexColor("#2b5f8a")

styles = getSampleStyleSheet()

# Custom styles
styles.add(ParagraphStyle(
    "DocTitle", parent=styles["Title"],
    fontSize=28, leading=34, textColor=DARK_BLUE,
    spaceAfter=4, alignment=TA_LEFT
))
styles.add(ParagraphStyle(
    "DocSubtitle", parent=styles["Normal"],
    fontSize=14, leading=18, textColor=MED_BLUE,
    spaceAfter=2, alignment=TA_LEFT
))
styles.add(ParagraphStyle(
    "TagLine", parent=styles["Normal"],
    fontSize=11, leading=15, textColor=MED_GRAY,
    spaceAfter=18, fontName="Helvetica-Oblique"
))
styles.add(ParagraphStyle(
    "SectionHead", parent=styles["Heading2"],
    fontSize=14, leading=18, textColor=DARK_BLUE,
    spaceBefore=14, spaceAfter=6,
    fontName="Helvetica-Bold"
))
styles.add(ParagraphStyle(
    "SubHead", parent=styles["Heading3"],
    fontSize=11, leading=14, textColor=MED_BLUE,
    spaceBefore=8, spaceAfter=4,
    fontName="Helvetica-Bold"
))
styles.add(ParagraphStyle(
    "Body", parent=styles["Normal"],
    fontSize=9, leading=12, textColor=DARK_GRAY,
    spaceAfter=6
))
styles.add(ParagraphStyle(
    "BodySmall", parent=styles["Normal"],
    fontSize=8, leading=10, textColor=DARK_GRAY,
    spaceAfter=4
))
styles.add(ParagraphStyle(
    "Quote", parent=styles["Normal"],
    fontSize=9.5, leading=13, textColor=DARK_BLUE,
    fontName="Helvetica-Oblique",
    leftIndent=20, rightIndent=20,
    spaceBefore=6, spaceAfter=10
))
styles.add(ParagraphStyle(
    "Feature", parent=styles["Normal"],
    fontSize=9, leading=12, textColor=DARK_GRAY,
    spaceAfter=2
))
styles.add(ParagraphStyle(
    "TableCell", parent=styles["Normal"],
    fontSize=8, leading=10, textColor=DARK_GRAY
))
styles.add(ParagraphStyle(
    "TableHeader", parent=styles["Normal"],
    fontSize=8, leading=10, textColor=white,
    fontName="Helvetica-Bold"
))
styles.add(ParagraphStyle(
    "Footer", parent=styles["Normal"],
    fontSize=7.5, leading=10, textColor=MED_GRAY,
    alignment=TA_CENTER
))
styles.add(ParagraphStyle(
    "VersionBadge", parent=styles["Normal"],
    fontSize=9, leading=12, textColor=MED_BLUE,
    fontName="Helvetica-Bold"
))


def make_table(headers, rows, col_widths=None):
    """Build a styled table."""
    hdr = [Paragraph(h, styles["TableHeader"]) for h in headers]
    data = [hdr]
    for row in rows:
        data.append([Paragraph(str(c), styles["TableCell"]) for c in row])

    t = Table(data, colWidths=col_widths, repeatRows=1)
    style_cmds = [
        ("BACKGROUND", (0, 0), (-1, 0), TABLE_HEADER),
        ("TEXTCOLOR", (0, 0), (-1, 0), white),
        ("FONTNAME", (0, 0), (-1, 0), "Helvetica-Bold"),
        ("FONTSIZE", (0, 0), (-1, 0), 8),
        ("BOTTOMPADDING", (0, 0), (-1, 0), 6),
        ("TOPPADDING", (0, 0), (-1, 0), 6),
        ("GRID", (0, 0), (-1, -1), 0.5, HexColor("#cccccc")),
        ("ROWBACKGROUNDS", (0, 1), (-1, -1), [white, LIGHT_GRAY]),
        ("VALIGN", (0, 0), (-1, -1), "TOP"),
        ("LEFTPADDING", (0, 0), (-1, -1), 6),
        ("RIGHTPADDING", (0, 0), (-1, -1), 6),
        ("TOPPADDING", (0, 1), (-1, -1), 4),
        ("BOTTOMPADDING", (0, 1), (-1, -1), 4),
    ]
    t.setStyle(TableStyle(style_cmds))
    return t


def feature_item(title, desc):
    return Paragraph(
        f"<b>{title}</b> &mdash; {desc}", styles["Feature"]
    )


def build_page1(story):
    """Page 1: Hero + key features."""
    story.append(Paragraph("v1.1.0", styles["VersionBadge"]))
    story.append(Spacer(1, 2))
    story.append(Paragraph("OpenLcbCLib", styles["DocTitle"]))
    story.append(Paragraph(
        "Full OpenLCB / LCC Protocol Stack in C", styles["DocSubtitle"]
    ))
    story.append(Paragraph(
        "Runs on any processor. No OS. No heap. Just clean C.",
        styles["TagLine"]
    ))

    # What is OpenLCB/LCC
    story.append(Paragraph("What is OpenLCB / LCC?", styles["SectionHead"]))
    story.append(Paragraph(
        "OpenLCB (Open Layout Control Bus) is an open, royalty-free standard "
        "for connecting model railroad accessories \u2014 throttles, signal towers, "
        "switch machines, lighting controllers, and more. The NMRA adopted it "
        "as LCC (Layout Command Control), making it the industry standard for "
        "interoperable layout electronics.",
        styles["Body"]
    ))
    story.append(Paragraph(
        "Nodes communicate over CAN bus or WiFi using a rich, well-specified "
        "protocol stack. Any manufacturer\u2019s node can talk to any other \u2014 "
        "out of the box, no configuration.",
        styles["Body"]
    ))

    # What is OpenLcbCLib
    story.append(Paragraph("What is OpenLcbCLib?", styles["SectionHead"]))
    story.append(Paragraph(
        "The full OpenLCB/LCC protocol stack, written in plain C.",
        styles["Body"]
    ))
    story.append(Paragraph(
        "\u201cNo dynamic memory. No OS. No external dependencies. If your chip "
        "has a C compiler and a CAN peripheral, it can run OpenLcbCLib.\u201d",
        styles["Quote"]
    ))

    # Key Features
    story.append(Paragraph("Key Features", styles["SectionHead"]))
    features = [
        ("Zero heap allocation",
         "Every buffer is a static array sized at compile time. No malloc, "
         "no fragmentation, no surprises."),
        ("Full consist forwarding",
         "Train Control listener management, P-bit forwarding, direction "
         "reversal, function linking, and source-skip loop prevention \u2014 "
         "all implemented."),
        ("OS-free by design",
         "No RTOS required. A single run() call per main-loop tick drives "
         "the entire protocol engine."),
        ("Compliance-tested",
         "45 gTest suites deliver 99.2% line, 99.9% function, and 95.8% "
         "branch coverage across the full source tree. All 93 OlcbChecker "
         "integration checks pass fully automated."),
        ("Dependency injection",
         "Hardware callbacks are function pointers wired at init time. Swap "
         "platforms by changing the driver layer \u2014 the protocol code is "
         "untouched."),
        ("Node Wizard",
         "Browser-based project generator. Opens offline from the repo. "
         "Pick your platform, configure your node, download a ready-to-build "
         "ZIP."),
        ("Virtual multi-node",
         "Host multiple logical nodes in one firmware image with full sibling "
         "loopback dispatch between them."),
        ("Stream Transport",
         "Full stream initiation, buffer negotiation, flow-controlled data "
         "transfer, and stream completion \u2014 including config memory over "
         "streams."),
        ("MIT-friendly license",
         "BSD 2-Clause. Use in open-source or commercial products with no "
         "royalties and no strings."),
    ]
    for title, desc in features:
        story.append(feature_item(title, desc))


def build_page2(story):
    """Page 2: Protocol coverage + platforms + architecture + coverage."""
    story.append(PageBreak())

    # Protocol Coverage
    story.append(Paragraph("Protocol Coverage", styles["SectionHead"]))
    proto_headers = ["Layer", "Details"]
    proto_rows = [
        ["Frame Layer",
         "Alias negotiation, conflict detection, CID/RID/AMD/AMR, CAN framing"],
        ["Message Network",
         "Verified Node ID, Protocol Support (PIP), OIR, Terminate Due to Error"],
        ["SNIP",
         "Simple Node Info Protocol \u2014 null-guard and null-terminator compliant"],
        ["Event Transport",
         "Producer / consumer, identify events, learn / teach"],
        ["Datagram",
         "Send, receive, timeout, 3-retry limit, temporary error backoff"],
        ["Config Memory",
         "Read, write, write-under-mask, lock, ACDI spaces, CDI / FDI delivery"],
        ["Broadcast Time",
         "Query, set, immediate time, rollover, producer and consumer roles"],
        ["Train Control",
         "Speed, functions, e-stop, heartbeat, consist management, controller assign"],
        ["Train Search",
         "Search by address / flags, dynamic node creation, multi-protocol dispatch"],
        ["Stream Transport",
         "Initiation, buffer negotiation, flow-controlled data transfer, "
         "completion, config memory over streams"],
        ["DCC Detector",
         "Track occupancy detection, event-based reporting, identify producer support"],
        ["Firmware Upgrade",
         "Freeze / unfreeze, firmware write, bootloader handoff"],
    ]
    story.append(make_table(proto_headers, proto_rows,
                            col_widths=[1.2 * inch, 5.0 * inch]))

    story.append(Spacer(1, 10))

    # Demo Platforms
    story.append(Paragraph(
        "Demo Platforms, Tools &amp; Getting Started", styles["SectionHead"]
    ))
    story.append(Paragraph(
        "Eight platforms ship with ready-to-run example projects. Porting to "
        "a new CAN platform means writing just two driver files \u2014 a CAN "
        "hardware driver and an OpenLCB driver \u2014 then everything else runs "
        "unchanged.",
        styles["Body"]
    ))

    plat_headers = ["Platform", "Transport", "Toolchain / IDE"]
    plat_rows = [
        ["ESP32", "CAN (TWAI)", "Arduino IDE, PlatformIO"],
        ["ESP32", "WiFi GridConnect", "Arduino IDE, PlatformIO"],
        ["Raspberry Pi Pico (RP2040)", "MCP2517FD SPI",
         "Arduino IDE (Philhower core)"],
        ["STM32F4xx", "CAN", "STM32CubeIDE"],
        ["TI MSPM0", "MCAN", "Code Composer Theia"],
        ["dsPIC", "CAN", "MPLAB X"],
        ["macOS", "GridConnect", "Xcode"],
    ]
    story.append(make_table(plat_headers, plat_rows,
                            col_widths=[1.8 * inch, 1.4 * inch, 2.6 * inch]))

    story.append(Spacer(1, 10))

    # Architecture highlights
    story.append(Paragraph("Architecture Highlights", styles["SubHead"]))
    arch_points = [
        "Static buffer pools sized at compile time \u2014 no malloc, no fragmentation",
        "Hardware access via nullable function-pointer interfaces (DI pattern)",
        "Single-tick main loop \u2014 no threads, no mutexes on single-core MCUs",
        "Alias validation: AMR scrub, FIFO invalidation, periodic re-verify",
    ]
    for pt in arch_points:
        story.append(Paragraph(f"\u2022 {pt}", styles["BodySmall"]))


def build_page3(story):
    """Page 3: Unit test coverage + getting started + docs links."""
    story.append(PageBreak())

    # Unit Test Coverage
    story.append(Paragraph("Unit Test Coverage", styles["SectionHead"]))
    cov_headers = ["Metric", "Coverage", "Detail"]
    cov_rows = [
        ["Lines", "99.2%", "8,047 / 8,109"],
        ["Functions", "99.9%", "674 / 675"],
        ["Branches", "95.8%", "2,918 / 3,047"],
    ]
    story.append(make_table(cov_headers, cov_rows,
                            col_widths=[1.2 * inch, 1.0 * inch, 2.0 * inch]))
    story.append(Spacer(1, 4))
    story.append(Paragraph(
        "45 gTest suites cover the full source tree (utilities, CAN driver, "
        "protocol engine). All 93 OlcbChecker integration checks pass "
        "(FR, ME, SN, EV, DA, MC, CD, FD, BT, TR, TS, ST, DD); "
        "all 93 run fully automated.",
        styles["BodySmall"]
    ))

    story.append(Spacer(1, 10))

    # Getting Started
    story.append(Paragraph("Getting Started", styles["SectionHead"]))
    steps = [
        ("<b>Quick Start (ESP32)</b> &mdash; Copy "
         "applications/arduino/esp32/BasicNode/ into your Arduino sketchbook "
         "and upload. Live on your layout in minutes."),
        ("<b>Node Wizard</b> &mdash; Open offline. Pick platform, configure "
         "node type and CDI, download a ready-to-build ZIP."),
        ("<b>New CAN platform</b> &mdash; Write a CAN hardware driver and "
         "an OpenLCB driver. The entire protocol engine runs unchanged."),
    ]
    for i, step in enumerate(steps, 1):
        story.append(Paragraph(f"{i}. {step}", styles["Body"]))

    story.append(Spacer(1, 10))

    # Documentation & Repository
    story.append(Paragraph(
        "Documentation &amp; Repository", styles["SectionHead"]
    ))
    docs = [
        "<b>Quick Start Guide</b> \u2014 Step-by-step ESP32 walkthrough",
        "<b>Developer Guide</b> \u2014 Node Wizard, callbacks, CDI, project layout",
        "<b>Implementation Guide</b> \u2014 State machines, protocol internals, architecture",
        "<b>API Reference</b> \u2014 Full Doxygen-generated HTML",
    ]
    for d in docs:
        story.append(Paragraph(d, styles["Body"]))
    story.append(Spacer(1, 6))
    story.append(Paragraph(
        "github.com/jimkueneman/OpenLcbCLib \u2014 BSD 2-Clause License",
        styles["Body"]
    ))
    story.append(Spacer(1, 16))
    story.append(Paragraph(
        "Built for model railroaders, by a model railroader.",
        styles["Quote"]
    ))


def add_footer(canvas_obj, doc):
    """Draw footer on every page."""
    canvas_obj.saveState()
    canvas_obj.setFont("Helvetica", 7.5)
    canvas_obj.setFillColor(MED_GRAY)
    w = letter[0]
    canvas_obj.drawCentredString(
        w / 2, 0.45 * inch,
        "OpenLcbCLib  |  Full Protocol Stack for OpenLCB / LCC"
    )
    canvas_obj.drawCentredString(
        w / 2, 0.3 * inch,
        "jimkueneman.github.io/OpenLcbCLib  \u2022  BSD 2-Clause  "
        "\u2022  Author: Jim Kueneman"
    )
    canvas_obj.restoreState()


def main():
    doc = SimpleDocTemplate(
        OUTPUT,
        pagesize=letter,
        topMargin=0.6 * inch,
        bottomMargin=0.7 * inch,
        leftMargin=0.75 * inch,
        rightMargin=0.75 * inch,
        title="OpenLcbCLib Brochure",
        author="Jim Kueneman",
    )

    story = []
    build_page1(story)
    build_page2(story)
    build_page3(story)

    doc.build(story, onFirstPage=add_footer, onLaterPages=add_footer)
    print(f"Created {OUTPUT}")


if __name__ == "__main__":
    main()
