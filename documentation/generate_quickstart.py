#!/usr/bin/env python3
"""Generate the OpenLcbCLib Quick Start Guide PDF with current project data."""

from reportlab.lib.pagesizes import letter
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.units import inch
from reportlab.lib.colors import HexColor, black, white
from reportlab.lib.enums import TA_LEFT, TA_CENTER
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle,
    PageBreak, Preformatted
)

OUTPUT = "QuickStartGuide.pdf"

DARK_BLUE = HexColor("#1a3a5c")
MED_BLUE = HexColor("#2b5f8a")
LIGHT_GRAY = HexColor("#f0f0f0")
DARK_GRAY = HexColor("#333333")
MED_GRAY = HexColor("#666666")
CODE_BG = HexColor("#f5f5f5")
TABLE_HEADER = HexColor("#2b5f8a")

styles = getSampleStyleSheet()

styles.add(ParagraphStyle("DocTitle", parent=styles["Title"],
    fontSize=24, leading=30, textColor=DARK_BLUE, spaceAfter=4, alignment=TA_LEFT))
styles.add(ParagraphStyle("DocSubtitle", parent=styles["Normal"],
    fontSize=12, leading=16, textColor=MED_BLUE, spaceAfter=2))
styles.add(ParagraphStyle("TagLine", parent=styles["Normal"],
    fontSize=10, leading=14, textColor=MED_GRAY, spaceAfter=14, fontName="Helvetica-Oblique"))
styles.add(ParagraphStyle("H1", parent=styles["Heading1"],
    fontSize=16, leading=20, textColor=DARK_BLUE, spaceBefore=16, spaceAfter=8, fontName="Helvetica-Bold"))
styles.add(ParagraphStyle("H2", parent=styles["Heading2"],
    fontSize=13, leading=17, textColor=MED_BLUE, spaceBefore=12, spaceAfter=6, fontName="Helvetica-Bold"))
styles.add(ParagraphStyle("H3", parent=styles["Heading3"],
    fontSize=11, leading=14, textColor=DARK_BLUE, spaceBefore=8, spaceAfter=4, fontName="Helvetica-Bold"))
styles.add(ParagraphStyle("Body", parent=styles["Normal"],
    fontSize=9, leading=12, textColor=DARK_GRAY, spaceAfter=6))
styles.add(ParagraphStyle("Note", parent=styles["Normal"],
    fontSize=8.5, leading=11, textColor=MED_BLUE, fontName="Helvetica-Oblique",
    leftIndent=14, spaceAfter=6, spaceBefore=4))
styles.add(ParagraphStyle("CodeBlock", fontName="Courier", fontSize=7.5, leading=9.5,
    textColor=DARK_GRAY, leftIndent=14, rightIndent=14, spaceAfter=6,
    backColor=CODE_BG, borderPadding=4))
styles.add(ParagraphStyle("TableCell", parent=styles["Normal"],
    fontSize=8, leading=10, textColor=DARK_GRAY))
styles.add(ParagraphStyle("TableHeader", parent=styles["Normal"],
    fontSize=8, leading=10, textColor=white, fontName="Helvetica-Bold"))
styles.add(ParagraphStyle("TOCEntry", parent=styles["Normal"],
    fontSize=9.5, leading=14, textColor=DARK_BLUE, fontName="Helvetica-Bold"))
styles.add(ParagraphStyle("Footer", parent=styles["Normal"],
    fontSize=7.5, leading=10, textColor=MED_GRAY, alignment=TA_CENTER))


def make_table(headers, rows, col_widths=None):
    hdr = [Paragraph(h, styles["TableHeader"]) for h in headers]
    data = [hdr]
    for row in rows:
        data.append([Paragraph(str(c), styles["TableCell"]) for c in row])
    t = Table(data, colWidths=col_widths, repeatRows=1)
    t.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, 0), TABLE_HEADER),
        ("TEXTCOLOR", (0, 0), (-1, 0), white),
        ("GRID", (0, 0), (-1, -1), 0.5, HexColor("#cccccc")),
        ("ROWBACKGROUNDS", (0, 1), (-1, -1), [white, LIGHT_GRAY]),
        ("VALIGN", (0, 0), (-1, -1), "TOP"),
        ("LEFTPADDING", (0, 0), (-1, -1), 5),
        ("RIGHTPADDING", (0, 0), (-1, -1), 5),
        ("TOPPADDING", (0, 0), (-1, -1), 4),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 4),
    ]))
    return t


def code_block(text):
    return Preformatted(text, styles["CodeBlock"])


def build(story):
    # Title
    story.append(Paragraph("OpenLcbCLib", styles["DocTitle"]))
    story.append(Paragraph("Quick Start Guide", styles["DocSubtitle"]))
    story.append(Paragraph("Get an OpenLCB/LCC node running in minutes", styles["TagLine"]))
    story.append(Paragraph("ESP32 + Arduino IDE edition", styles["TagLine"]))
    story.append(Paragraph(
        "OpenLcbCLib supports many platforms \u2014 this guide covers one specific path.",
        styles["Note"]))

    # TOC
    story.append(PageBreak())
    story.append(Paragraph("Table of Contents", styles["H1"]))
    toc = [
        "1. What is OpenLCB/LCC?",
        "2. What You Need",
        "3. Project Setup for Arduino IDE",
        "4. Choosing a Node ID",
        "5. Starting a Project From Scratch",
        "6. Understanding openlcb_user_config.h",
        "7. Uploading and Running",
        "8. What\u2019s Next",
    ]
    for entry in toc:
        story.append(Paragraph(entry, styles["TOCEntry"]))

    # 1. What is OpenLCB/LCC
    story.append(PageBreak())
    story.append(Paragraph("1. What is OpenLCB/LCC?", styles["H1"]))
    story.append(Paragraph(
        "Note: This guide uses an ESP32 development board with Arduino IDE as a concrete, working example. "
        "OpenLcbCLib itself is platform-agnostic C \u2014 it can be adapted to any microcontroller with a C "
        "compiler. See the Developer Guide for other platforms and IDEs.", styles["Note"]))
    story.append(Paragraph(
        "OpenLCB (Open Layout Control Bus) is an open-source project that produced a suite of specifications "
        "and technical notes for connecting model railroad devices. The NMRA adopted these specifications "
        "and technical notes as the LCC (Layout Command Control) standard. LCC and OpenLCB refer to the "
        "same protocol \u2014 LCC is simply the NMRA\u2019s official name for it.", styles["Body"]))
    story.append(Paragraph(
        "Throttles, signal controllers, turnout drivers, occupancy detectors, and other accessories all talk "
        "to each other over a shared CAN bus (or WiFi) using a common language. OpenLcbCLib is a portable C "
        "library that implements these protocols for microcontrollers, handling all low-level protocol work so "
        "you can focus on what your node actually does.", styles["Body"]))

    story.append(Paragraph("Key Terms", styles["H2"]))
    terms = [
        ("<b>Node:</b>", "Any device on the network. Your ESP32 board is a node."),
        ("<b>Node ID:</b>", "A unique 48-bit number that identifies your node globally."),
        ("<b>Event ID:</b>", "A 64-bit number representing something that happened or a command to perform."),
        ("<b>Producer:</b>", "A node that sends an event when something happens."),
        ("<b>Consumer:</b>", "A node that reacts to an event."),
        ("<b>CDI:</b>", "Configuration Description Information \u2014 an XML file describing what settings your node exposes."),
        ("<b>CAN Bus:</b>", "The physical wiring (two wires) that connects all nodes on the layout."),
    ]
    for term, desc in terms:
        story.append(Paragraph(f"\u2022 {term} {desc}", styles["Body"]))

    # 2. What You Need
    story.append(Paragraph("2. What You Need", styles["H1"]))
    story.append(Paragraph("Hardware", styles["H2"]))
    story.append(Paragraph("\u2022 ESP32 development board (any common 38-pin or 30-pin board)", styles["Body"]))
    story.append(Paragraph("\u2022 CAN bus transceiver module (SN65HVD230 or MCP2551 recommended)", styles["Body"]))
    story.append(Paragraph("\u2022 USB cable for programming", styles["Body"]))
    story.append(Paragraph("\u2022 LCC/OpenLCB layout bus, OR two ESP32 boards to test with", styles["Body"]))
    story.append(Paragraph(
        "Note: The ESP32 has a built-in CAN controller called TWAI (Two-Wire Automotive Interface). "
        "You only need an external transceiver chip \u2014 you do NOT need a separate CAN controller.", styles["Note"]))

    story.append(Paragraph("CAN Transceiver Wiring", styles["H3"]))
    story.append(make_table(
        ["ESP32 Pin", "Transceiver Pin"],
        [
            ["GPIO 21", "TX (CAN transmit)"],
            ["GPIO 22", "RX (CAN receive)"],
            ["3.3V or 5V", "VCC (check your module datasheet)"],
            ["GND", "GND"],
            ["(layout bus)", "CANH and CANL to bus wires"],
        ],
        col_widths=[2.0*inch, 4.0*inch]
    ))

    story.append(Paragraph("Software", styles["H2"]))
    story.append(Paragraph("\u2022 Arduino IDE 2.x or PlatformIO (VSCode)", styles["Body"]))
    story.append(Paragraph("\u2022 ESP32 Arduino core installed in Arduino IDE (search \u201cesp32\u201d in Boards Manager)", styles["Body"]))
    story.append(Paragraph("\u2022 A modern web browser (Chrome, Firefox, or Edge) for the Node Wizard", styles["Body"]))
    story.append(Paragraph("\u2022 The OpenLcbCLib source code", styles["Body"]))

    # 3. Project Setup
    story.append(PageBreak())
    story.append(Paragraph("3. Project Setup for Arduino IDE", styles["H1"]))
    story.append(Paragraph(
        "The library is NOT installed as an Arduino Library. Instead, the source files live directly inside "
        "your sketch folder. This keeps every project self-contained with its own copy of the library.",
        styles["Body"]))
    story.append(Paragraph(
        "Important Arduino IDE rule: Arduino IDE will only automatically compile C/C++ files that are in the "
        "sketch folder itself or in a subfolder named exactly src. Files in any other subfolder are ignored.",
        styles["Note"]))

    story.append(Paragraph("Starting From the ESP32 Example", styles["H2"]))
    story.append(Paragraph("The library includes a ready-to-run ESP32 example at:", styles["Body"]))
    story.append(code_block("OpenLcbCLib/applications/arduino/esp32/BasicNode/"))
    story.append(Paragraph(
        "1. Copy the entire BasicNode folder to your Arduino sketches folder.", styles["Body"]))
    story.append(Paragraph(
        "2. Rename the folder to your project name, then rename the .ino file inside it to the same name. "
        "Arduino IDE requires the folder and .ino file to have exactly the same name.", styles["Body"]))
    story.append(Paragraph("3. Open the renamed .ino file in Arduino IDE.", styles["Body"]))

    story.append(Paragraph("Complete Project File Structure", styles["H2"]))
    story.append(code_block(
"""BasicNode/                    <- your sketch folder
  BasicNode.ino               <- main sketch
  openlcb_user_config.h       <- buffer sizes & feature flags
  openlcb_user_config.c       <- node parameter data
  src/                        <- Arduino IDE compiles this subtree
    application_callbacks/    <- your application logic
      callbacks_can.h / .cpp
      callbacks_olcb.h / .cpp
      callbacks_config_mem.h / .cpp
    application_drivers/      <- hardware interface
      esp32_can_drivers.h / .cpp
      esp32_drivers.h / .cpp
    openlcb_c_lib/            <- library (do not edit)
      openlcb/                <- protocol engine
      drivers/canbus/         <- CAN state machines
      utilities/              <- helper files"""))
    story.append(Paragraph(
        "Note: Never rename the src/ subfolder. Arduino IDE will stop compiling the library files if "
        "the folder has any other name.", styles["Note"]))

    # 4. Choosing a Node ID
    story.append(PageBreak())
    story.append(Paragraph("4. Choosing a Node ID", styles["H1"]))
    story.append(Paragraph(
        "Every node on a layout must have a unique 48-bit Node ID \u2014 think of it like a MAC address. "
        "If two nodes share an ID the network will have conflicts.", styles["Body"]))

    story.append(Paragraph("Option A: NMRA Member Range", styles["H2"]))
    story.append(Paragraph(
        "If you are an NMRA member, you automatically have a personal Node ID range derived from your "
        "member number. Log in to the NMRA member portal to look up your assigned range.", styles["Body"]))
    story.append(Paragraph("Option B: Request a Manufacturer Range", styles["H2"]))
    story.append(Paragraph(
        "If you are producing nodes commercially or distributing firmware publicly, request a dedicated "
        "range through the OpenLCB group. See the registry at registry.openlcb.org/uniqueidranges.", styles["Body"]))
    story.append(Paragraph("Option C: Development / Test Range", styles["H2"]))
    story.append(Paragraph(
        "For personal testing on your own layout, you can use Node IDs from the reserved development range. "
        "These are safe to use privately but should not be used in nodes you distribute to others.", styles["Body"]))

    story.append(Paragraph("Setting the Node ID", styles["H2"]))
    story.append(Paragraph(
        "BasicNode demo \u2014 the Node ID is defined as a constant at the top of the sketch:", styles["Body"]))
    story.append(code_block(
"""// In BasicNode.ino, find and change this line:
#define NODE_ID 0x050101010777

// Use any value in the development range:
#define NODE_ID 0x050101010001  // node 1
#define NODE_ID 0x050101010002  // node 2
// Keep a list - every node needs a different ID"""))
    story.append(Paragraph(
        "Wizard-generated projects \u2014 enter your Node ID in the Project Options panel before you generate "
        "the code. The Wizard accepts the ID in dotted hex format (xx.xx.xx.xx.xx.xx) and writes it into "
        "the generated files automatically.", styles["Body"]))
    story.append(Paragraph(
        "Note: Every node on your layout must have a DIFFERENT Node ID, even when using the development "
        "range. Keep a list so you do not accidentally reuse one.", styles["Note"]))

    # 5. Starting a Project From Scratch
    story.append(PageBreak())
    story.append(Paragraph("5. Starting a Project From Scratch", styles["H1"]))
    story.append(Paragraph(
        "Once you are comfortable with the BasicNode demo, you will want to start your own project. Open "
        "the browser-based Node Wizard at tools/node_wizard/node_wizard.html in any modern browser \u2014 "
        "no internet connection required. Work through the sidebar from top to bottom:", styles["Body"]))
    story.append(make_table(
        ["Sidebar Section", "What You Do There"],
        [
            ["Node Type", "Pick the role of your node: Basic, Typical, Train, or Train Controller."],
            ["CDI", "Describe the settings your node exposes to JMRI and other tools. Required for Typical, Train, and Train Controller nodes."],
            ["FDI", "Describe the DCC functions your decoder supports. Train nodes only."],
            ["Platform Drivers", "Select your target hardware. The Wizard pre-fills working driver code for the chosen platform."],
            ["Callbacks", "Choose which protocol events your application needs to respond to."],
            ["Generated Files", "Preview the output file tree and click Download ZIP to get your project."],
        ],
        col_widths=[1.3*inch, 4.9*inch]
    ))
    story.append(Spacer(1, 8))
    story.append(make_table(
        ["Platform", "Framework Layout"],
        [
            ["ESP32 + TWAI", "Arduino / PlatformIO"],
            ["ESP32 + WiFi GridConnect", "Arduino / PlatformIO"],
            ["RP2040 + MCP2517FD", "Arduino (Earle Philhower core)"],
            ["STM32F4xx + CAN", "STM32 HAL (CubeIDE)"],
            ["TI MSPM0 + MCAN", "TI DriverLib (CCS / Theia)"],
            ["None / Custom", "Your own toolchain"],
        ],
        col_widths=[2.5*inch, 3.0*inch]
    ))
    story.append(Paragraph(
        "Note: The Developer Guide covers the Node Wizard in full detail, including every section, "
        "the CDI editor, and the difference between Arduino and non-Arduino folder layouts.", styles["Note"]))

    # 6. Understanding openlcb_user_config.h
    story.append(PageBreak())
    story.append(Paragraph("6. Understanding openlcb_user_config.h", styles["H1"]))
    story.append(Paragraph(
        "This is the most important file in your project. It tells the library how much memory to allocate "
        "and which features to compile in. Every value is mandatory.", styles["Body"]))
    story.append(code_block(
"""// --- Feature Flags -------------------------------------------
// Uncomment to enable the protocols your node uses.
#define OPENLCB_COMPILE_EVENTS                // event producer/consumer
#define OPENLCB_COMPILE_DATAGRAMS             // needed for config memory
#define OPENLCB_COMPILE_MEMORY_CONFIGURATION  // CDI/settings support
// #define OPENLCB_COMPILE_FIRMWARE           // firmware upgrade
// #define OPENLCB_COMPILE_STREAM             // stream transport
// #define OPENLCB_COMPILE_BROADCAST_TIME     // clock synchronization
// #define OPENLCB_COMPILE_TRAIN             // locomotive control
// #define OPENLCB_COMPILE_TRAIN_SEARCH      // throttle train discovery
// #define OPENLCB_COMPILE_DCC_DETECTOR      // DCC detection protocol

// --- Buffer Pool --------------------------------------------
#define USER_DEFINED_BASIC_BUFFER_DEPTH      32  // 16 bytes each
#define USER_DEFINED_DATAGRAM_BUFFER_DEPTH    4  // 72 bytes each
#define USER_DEFINED_SNIP_BUFFER_DEPTH        4  // 256 bytes each
#define USER_DEFINED_STREAM_BUFFER_DEPTH      1  // stream data

// --- Node Count ---------------------------------------------
#define USER_DEFINED_NODE_BUFFER_DEPTH        4

// --- Events -------------------------------------------------
#define USER_DEFINED_PRODUCER_COUNT          64
#define USER_DEFINED_CONSUMER_COUNT          64"""))
    story.append(Paragraph(
        "Note: The Node Wizard generates this file for you. The BasicNode example already includes it "
        "with sensible defaults for ESP32.", styles["Note"]))

    # 7. Uploading and Running
    story.append(Paragraph("7. Uploading and Running", styles["H1"]))
    story.append(Paragraph("In Arduino IDE", styles["H2"]))
    story.append(Paragraph("1. Open BasicNode.ino in Arduino IDE.", styles["Body"]))
    story.append(Paragraph('2. From Tools, select your board: "ESP32 Dev Module".', styles["Body"]))
    story.append(Paragraph("3. Select the correct COM port.", styles["Body"]))
    story.append(Paragraph(
        "4. BasicNode demo: change the #define NODE_ID at the top of the sketch to your chosen value "
        "(see Section 4). Wizard-generated project: the Node ID was already set in Project Options \u2014 "
        "nothing to change here.", styles["Body"]))
    story.append(Paragraph("5. Click Upload.", styles["Body"]))

    story.append(Paragraph("What Happens at Power-On", styles["H2"]))
    story.append(Paragraph(
        "When the node starts it goes through the LCC Alias Negotiation process. After a few seconds it "
        "becomes active and is visible to JMRI and other LCC tools on the bus.", styles["Body"]))

    story.append(Paragraph("Testing With JMRI", styles["H2"]))
    story.append(Paragraph("\u2022 Open JMRI and connect to your LCC network.", styles["Body"]))
    story.append(Paragraph(
        '\u2022 Go to Tools > LCC > LCC Nodes. Your node should appear in the list.', styles["Body"]))
    story.append(Paragraph(
        '\u2022 Right-click the node and choose "Configure". JMRI reads the CDI and shows your settings.',
        styles["Body"]))
    story.append(Paragraph(
        "\u2022 You can change the node name and description from here.", styles["Body"]))
    story.append(Paragraph(
        "Note: The BasicNode demo does not implement real persistent storage. Changes you make to the "
        "node name and description through JMRI will not survive a power cycle. How you store configuration "
        "data is up to you \u2014 common choices include external EEPROM, an SD card, or the ESP32\u2019s internal "
        "flash (NVS/Preferences). The Developer Guide covers how to wire your storage choice into the "
        "config memory callbacks.", styles["Note"]))

    # 8. What's Next
    story.append(PageBreak())
    story.append(Paragraph("8. What\u2019s Next", styles["H1"]))
    story.append(Paragraph(
        "You now have a working OpenLCB/LCC node. Here are the typical next steps:", styles["Body"]))

    story.append(Paragraph("Add Your Own Producer Events", styles["H2"]))
    story.append(code_block(
"""// In CallbacksOlcb_initialize():
openlcb_node_t *node = OpenLcbNode_get_first(0);
OpenLcbApplication_register_producer_eventid(
    node, 0x0501010107770001, EVENT_STATUS_CLEAR);

// In CallbacksOlcb_on_100ms_timer():
if (button_is_pressed())
    OpenLcbApplication_send_event_pc_report(
        node, 0x0501010107770001);"""))

    story.append(Paragraph("React to Consumer Events", styles["H2"]))
    story.append(code_block(
"""// Register the event to listen for:
OpenLcbApplication_register_consumer_eventid(
    node, 0x0501010107770002, EVENT_STATUS_UNKNOWN);

// In your on_consumed_event_pcer callback:
void my_event_handler(openlcb_node_t *node,
    uint16_t index, event_id_t *event_id,
    event_payload_t *payload)
{
    if (*event_id == 0x0501010107770002)
        turn_on_led();
}"""))

    story.append(Paragraph("Read the Developer Guide", styles["H2"]))
    story.append(Paragraph(
        "The companion document, OpenLcbCLib Developer Guide, walks through every part of the system in "
        "detail: the Node Wizard, drivers, callbacks, CDI configuration, and more.", styles["Body"]))

    story.append(Paragraph("Online Help", styles["H2"]))
    story.append(Paragraph(
        "Full documentation: https://jimkueneman.github.io/OpenLcbCLib/documentation/help/html/",
        styles["Body"]))


PAGE_NUM = [0]

def footer(canvas_obj, doc):
    PAGE_NUM[0] += 1
    canvas_obj.saveState()
    canvas_obj.setFont("Helvetica", 7.5)
    canvas_obj.setFillColor(MED_GRAY)
    w = letter[0]
    canvas_obj.drawString(0.75*inch, 0.4*inch, "OpenLcbCLib | Quick Start Guide")
    canvas_obj.drawRightString(w - 0.75*inch, 0.4*inch, f"Page {PAGE_NUM[0]}")
    canvas_obj.restoreState()


def main():
    doc = SimpleDocTemplate(
        OUTPUT, pagesize=letter,
        topMargin=0.6*inch, bottomMargin=0.65*inch,
        leftMargin=0.75*inch, rightMargin=0.75*inch,
        title="OpenLcbCLib Quick Start Guide",
        author="Jim Kueneman",
    )
    story = []
    build(story)
    doc.build(story, onFirstPage=footer, onLaterPages=footer)
    print(f"Created {OUTPUT}")


if __name__ == "__main__":
    main()
