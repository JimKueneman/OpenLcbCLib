#!/usr/bin/env python3
"""Generate the OpenLcbCLib Developer Guide PDF with current project data."""

from reportlab.lib.pagesizes import letter
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.units import inch
from reportlab.lib.colors import HexColor, black, white
from reportlab.lib.enums import TA_LEFT, TA_CENTER
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle,
    PageBreak, KeepTogether, Preformatted
)

OUTPUT = "DeveloperGuide.pdf"

# Colors
DARK_BLUE = HexColor("#1a3a5c")
MED_BLUE = HexColor("#2b5f8a")
LIGHT_BLUE = HexColor("#e8f0f8")
DARK_GRAY = HexColor("#333333")
MED_GRAY = HexColor("#666666")
LIGHT_GRAY = HexColor("#f0f0f0")
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
styles.add(ParagraphStyle("BodySmall", parent=styles["Normal"],
    fontSize=8, leading=10, textColor=DARK_GRAY, spaceAfter=4))
styles.add(ParagraphStyle("Note", parent=styles["Normal"],
    fontSize=8.5, leading=11, textColor=MED_BLUE, fontName="Helvetica-Oblique",
    leftIndent=14, spaceAfter=6, spaceBefore=4))
styles.add(ParagraphStyle("CodeBlock", fontName="Courier", fontSize=7.5, leading=9.5,
    textColor=DARK_GRAY, leftIndent=14, rightIndent=14, spaceAfter=6,
    backColor=CODE_BG, borderPadding=4))
styles.add(ParagraphStyle("TableCell", parent=styles["Normal"],
    fontSize=7.5, leading=10, textColor=DARK_GRAY))
styles.add(ParagraphStyle("TableHeader", parent=styles["Normal"],
    fontSize=7.5, leading=10, textColor=white, fontName="Helvetica-Bold"))
styles.add(ParagraphStyle("TOCEntry", parent=styles["Normal"],
    fontSize=9, leading=14, textColor=DARK_GRAY, leftIndent=10))
styles.add(ParagraphStyle("TOCSection", parent=styles["Normal"],
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
        ("FONTNAME", (0, 0), (-1, 0), "Helvetica-Bold"),
        ("FONTSIZE", (0, 0), (-1, 0), 7.5),
        ("BOTTOMPADDING", (0, 0), (-1, 0), 5),
        ("TOPPADDING", (0, 0), (-1, 0), 5),
        ("GRID", (0, 0), (-1, -1), 0.5, HexColor("#cccccc")),
        ("ROWBACKGROUNDS", (0, 1), (-1, -1), [white, LIGHT_GRAY]),
        ("VALIGN", (0, 0), (-1, -1), "TOP"),
        ("LEFTPADDING", (0, 0), (-1, -1), 5),
        ("RIGHTPADDING", (0, 0), (-1, -1), 5),
        ("TOPPADDING", (0, 1), (-1, -1), 3),
        ("BOTTOMPADDING", (0, 1), (-1, -1), 3),
    ]))
    return t


def code_block(text):
    return Preformatted(text, styles["CodeBlock"])


def build(story):
    # =========================================================================
    # TITLE PAGE
    # =========================================================================
    story.append(Paragraph("OpenLcbCLib", styles["DocTitle"]))
    story.append(Paragraph("Developer Guide", styles["DocSubtitle"]))
    story.append(Paragraph(
        "From first project to a fully featured OpenLCB/LCC node", styles["TagLine"]))
    story.append(Paragraph("Covers all supported platforms and IDEs", styles["TagLine"]))

    # =========================================================================
    # TABLE OF CONTENTS
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("Table of Contents", styles["H1"]))

    toc = [
        ("1.", "Introduction"),
        ("2.", "OpenLCB/LCC Concepts"),
        ("3.", "Project File Structure"),
        ("4.", "The Node Wizard in Detail"),
        ("", "4.1 Node Type Selection"),
        ("", "4.2 CDI Editor"),
        ("", "4.3 FDI Editor (Train Nodes Only)"),
        ("", "4.4 Driver Stubs"),
        ("", "4.5 Callback Stubs"),
        ("", "4.6 Exporting the Code"),
        ("5.", "openlcb_user_config.h in Depth"),
        ("", "5.1 Feature Flags"),
        ("", "5.2 Message Buffer Pool"),
        ("", "5.3 Node Count"),
        ("", "5.4 Event Counts"),
        ("", "5.5 Stream Transport"),
        ("", "5.6 Train Protocol"),
        ("", "5.7 Listener Alias Verification"),
        ("6.", "Initialization \u2014 main.ino"),
        ("", "6.1 The CAN Config Struct"),
        ("", "6.2 The OpenLCB Config Struct"),
        ("", "6.3 Setup and Loop"),
        ("7.", "Implementing the Drivers"),
        ("", "7.1 CAN Bus Drivers"),
        ("", "7.2 OpenLCB Platform Drivers"),
        ("8.", "Callbacks \u2014 Where Your Application Lives"),
        ("9.", "Events in Depth"),
        ("10.", "Configuration Memory"),
        ("11.", "Train Nodes (Brief Overview)"),
        ("12.", "Using PlatformIO"),
        ("13.", "Troubleshooting"),
    ]
    for num, title in toc:
        if num:
            story.append(Paragraph(f"{num} {title}", styles["TOCSection"]))
        else:
            story.append(Paragraph(title, styles["TOCEntry"]))

    # =========================================================================
    # 1. INTRODUCTION
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("1. Introduction", styles["H1"]))
    story.append(Paragraph(
        "OpenLcbCLib is a C library that implements the OpenLCB/LCC (Layout Command Control) protocol "
        "suite for microcontrollers. It is designed to be portable across any hardware platform and IDE, "
        "with no dynamic memory allocation \u2014 all buffers are statically sized at compile time, making it "
        "suitable for processors with very limited RAM.", styles["Body"]))
    story.append(Paragraph(
        "This guide walks you through creating a complete node project from scratch using the Node Wizard "
        "tool, then explains every generated file so you understand exactly what the code does and how to "
        "extend it.", styles["Body"]))

    story.append(Paragraph("What the Library Does (and Does Not Do)", styles["H2"]))
    story.append(Paragraph(
        "The library handles all OpenLCB/LCC protocol compliance: alias negotiation, node initialization, "
        "event transport, datagram handling, SNIP, configuration memory access, stream transport, "
        "and train control. It does NOT include any hardware-specific code. You supply short driver "
        "functions that read and write your CAN hardware, and the library calls them as needed.", styles["Body"]))

    story.append(Paragraph("Platform Support", styles["H2"]))
    story.append(Paragraph(
        "Because the library contains no hardware-specific code, it can be ported to any processor and "
        "toolchain. You provide a small set of driver functions for your CAN hardware and platform services, "
        "and the library does the rest. The following processors already have driver code written and "
        "example projects included in the library:", styles["Body"]))
    story.append(make_table(
        ["Processor / Board", "Example IDE / Toolchain"],
        [
            ["ESP32", "Arduino IDE, PlatformIO"],
            ["Raspberry Pi Pico (RP2040)", "Arduino IDE, PlatformIO"],
            ["STM32 (F4 and others)", "STM32 Cube IDE"],
            ["Microchip dsPIC33", "MPLAB X"],
            ["TI MSPM0", "Code Composer Studio"],
            ["macOS (simulation)", "Xcode"],
        ],
        col_widths=[2.5*inch, 3.5*inch]
    ))
    story.append(Spacer(1, 6))
    story.append(Paragraph(
        "Porting to a new processor means writing two short driver files (CAN hardware and platform "
        "services). The Node Wizard generates TODO-marked stubs for both so you know exactly what "
        "functions need to be filled in. Section 7 covers this in detail.", styles["Body"]))
    story.append(Paragraph(
        "This guide focuses on ESP32 as the worked example. All concepts apply equally to any other target.",
        styles["Note"]))

    # =========================================================================
    # 2. OPENLCB/LCC CONCEPTS
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("2. OpenLCB/LCC Concepts", styles["H1"]))
    story.append(Paragraph(
        "You do not need to read the full OpenLCB/LCC standards to use this library, but a basic "
        "understanding of the following concepts will help you make good design decisions.", styles["Body"]))

    story.append(Paragraph("The Network", styles["H2"]))
    story.append(Paragraph(
        "All LCC nodes connect to a shared CAN bus (or a TCP/IP gateway). Every node can hear every "
        "message on the bus. Nodes identify themselves with a globally unique 48-bit Node ID.", styles["Body"]))

    story.append(Paragraph("Events (Producer-Consumer Model)", styles["H2"]))
    story.append(Paragraph(
        "The most common way nodes communicate is through Events. An event is a 64-bit number that means "
        "\u201csomething happened.\u201d A node that detects a condition (button pressed, train entered block) fires "
        "an event. Any node that cares about that event reacts to it (throws a switch, changes a signal). "
        "The producer and consumer do not know about each other \u2014 they are loosely coupled through the "
        "shared event ID.", styles["Body"]))

    story.append(Paragraph("Configuration (CDI)", styles["H2"]))
    story.append(Paragraph(
        "A Typical node exposes configurable settings to the layout operator via CDI (Configuration "
        "Description Information). The CDI is an XML document stored in the node that describes what "
        "settings are available. JMRI and other tools read the CDI and show a configuration panel.", styles["Body"]))

    story.append(Paragraph("Node Types and What They Compile", styles["H2"]))
    story.append(make_table(
        ["Node Type", "Protocols Compiled", "Typical Use"],
        [
            ["Basic", "Events only", "Simple sensor or output that only fires/reacts to events."],
            ["Typical", "Events + Datagrams + Config Memory", "Most common. Sensor/output with user-configurable event IDs."],
            ["Train", "Typical + Train + FDI", "Locomotive decoder. Responds to speed/function commands."],
            ["Train Controller", "Typical + Train + Train Search", "Throttle or command station. Commands trains."],
            ["Bootloader", "Datagrams + Config Memory + Firmware", "Firmware upgrade image. Minimal protocol footprint."],
        ],
        col_widths=[1.1*inch, 2.0*inch, 3.1*inch]
    ))

    # =========================================================================
    # 3. PROJECT FILE STRUCTURE
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("3. Project File Structure", styles["H1"]))
    story.append(Paragraph(
        "Understanding the file layout tells you where to look when you need to change something. "
        "The files in src/openlcb/ and src/drivers/ are library code \u2014 do not edit them. "
        "The files at the top level are yours.", styles["Body"]))
    story.append(code_block(
"""YourProject/                  <- Arduino sketch folder
  YourProject.ino             <- Main sketch (setup + loop)
  openlcb_user_config.h       <- REQUIRED: buffer sizes & feature flags
  openlcb_user_config.c       <- User node parameter data
  src/
    application_callbacks/
      callbacks_can.h / .cpp   <- CAN observation hooks
      callbacks_olcb.h / .cpp  <- Application logic hooks
      callbacks_config_mem.h / .cpp <- Config memory hooks
    application_drivers/
      esp32_can_drivers.h / .cpp   <- CAN hardware interface
      esp32_drivers.h / .cpp       <- Platform services (NVS, mutex, reboot)
    xml_files/
      cdi.xml                  <- CDI definition
      fdi.xml                  <- FDI definition (train nodes only)
    openlcb_c_lib/             <- Library core (do not edit)
      openlcb/                 <- Protocol engine
        openlcb_config.h/c            - Stack init API
        openlcb_types.h               - All data types
        openlcb_defines.h             - Version and constants
        openlcb_node.h/c              - Node allocation and management
        openlcb_application.h/c       - Event registration/sending API
        openlcb_application_broadcast_time.h/c
        openlcb_application_train.h/c
        openlcb_application_dcc_detector.h/c
        openlcb_buffer_fifo.h/c       - FIFO buffer management
        openlcb_buffer_list.h/c       - List buffer management
        openlcb_buffer_store.h/c      - Buffer pool allocation
        openlcb_float16.h/c           - Float16 conversion
        openlcb_gridconnect.h/c       - GridConnect framing
        openlcb_utilities.h/c         - Helper functions
        openlcb_login_statemachine.h/c
        openlcb_login_statemachine_handler.h/c
        openlcb_main_statemachine.h/c
        protocol_event_transport.h/c  - Event protocol engine
        protocol_datagram_handler.h/c - Datagram protocol engine
        protocol_config_mem_read_handler.h/c
        protocol_config_mem_write_handler.h/c
        protocol_config_mem_operations_handler.h/c
        protocol_config_mem_stream_handler.h/c
        protocol_stream_handler.h/c   - Stream transport engine
        protocol_snip.h/c             - Node identification protocol
        protocol_message_network.h/c  - Message network handler
        protocol_broadcast_time_handler.h/c
        protocol_train_handler.h/c
        protocol_train_search_handler.h/c
      drivers/canbus/          <- CAN state machines (do not edit)
      utilities/               <- Shared utilities"""))

    # =========================================================================
    # 4. NODE WIZARD
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("4. The Node Wizard in Detail", styles["H1"]))
    story.append(Paragraph(
        "The Node Wizard is a browser-based code generator at tools/node_wizard/node_wizard.html. "
        "Open it in any modern browser \u2014 no installation or internet connection needed. Work through "
        "the sidebar sections from top to bottom. All settings persist automatically in your browser\u2019s "
        "local storage. Use Save Project / Load Project in the header to save and reload a project "
        "across browsers or machines.", styles["Body"]))

    story.append(Paragraph("4.1 Node Type and Project Options", styles["H2"]))
    story.append(Paragraph(
        "The first step is selecting the role of your node. This determines which feature flags are set "
        "in openlcb_user_config.h, which sidebar sections are enabled, and which callback stubs are generated.",
        styles["Body"]))
    story.append(make_table(
        ["Node Type", "Description"],
        [
            ["Basic", "Sends and receives events only. No configurable settings. CDI and FDI sections are disabled."],
            ["Typical", "Events plus user-configurable settings stored in config memory. The CDI section is enabled. Most common choice for sensors, turnout drivers, and signal heads."],
            ["Train", "Locomotive decoder. CDI and FDI sections are both enabled."],
            ["Train Controller", "Throttle or command station."],
            ["Custom", "Select your own feature flags and callbacks manually."],
        ],
        col_widths=[1.2*inch, 5.0*inch]
    ))
    story.append(Spacer(1, 6))
    story.append(Paragraph("Project Options", styles["H3"]))
    story.append(Paragraph(
        "Below the node type buttons, the Project Options panel is always visible. Fill this in before "
        "generating \u2014 the values flow into the generated code and the ZIP filename.", styles["Body"]))
    story.append(make_table(
        ["Field", "Description"],
        [
            ["Project Name", "Short identifier for your project, e.g. my_signal_controller. Used as the ZIP filename when you download. Stick to letters, numbers, and underscores."],
            ["Author", "Your name or organisation. Written into the generated file headers."],
            ["Node ID", "Your node's globally unique 48-bit identifier in dotted hex format: xx.xx.xx.xx.xx.xx (e.g. 05.01.01.00.00.01). The field auto-formats as you type and shows a live valid / invalid indicator."],
            ["Broadcast Time", "Optionally compile in the OpenLCB Broadcast Time protocol. Choose None (default), Producer, or Consumer."],
            ["Firmware Update", "Enables the OpenLCB Firmware Upgrade Protocol so the node can accept a new firmware image over the bus without physical access. Requires config memory."],
        ],
        col_widths=[1.2*inch, 5.0*inch]
    ))

    # 4.2 CDI Editor
    story.append(Paragraph("4.2 CDI Editor", styles["H2"]))
    story.append(Paragraph(
        "The CDI editor lets you describe what settings your node exposes to configuration tools like JMRI. "
        "The CDI is an XML document. The editor provides a visual interface so you do not need to write raw XML. "
        "This section is enabled for Typical, Train, and Train Controller nodes.", styles["Body"]))
    story.append(Paragraph("CDI Structure", styles["H3"]))
    story.append(Paragraph(
        "A CDI document has two main parts: an identification block (manufacturer, model, versions) and one or "
        "more configuration segments. A segment is a named block of settings stored at a specific memory address.",
        styles["Body"]))
    story.append(make_table(
        ["CDI Element", "Description"],
        [
            ["&lt;string size=\"N\"&gt;", "Text field N bytes long. Used for names and descriptions."],
            ["&lt;eventid&gt;", "Stores a 64-bit event ID (8 bytes). Most common field type."],
            ["&lt;int size=\"N\"&gt;", "Integer value N bytes (1, 2, or 4)."],
            ["&lt;group replication=\"N\"&gt;", "Repeats the group N times. Useful for multi-channel nodes."],
            ["&lt;segment space=\"253\"&gt;", "Space 253 is the user configuration space JMRI reads/writes."],
        ],
        col_widths=[1.6*inch, 4.6*inch]
    ))
    story.append(Paragraph(
        "Important: The total bytes in your CDI segments must match the actual memory layout in your "
        "config_mem_read/write driver functions. Mismatches cause JMRI to read garbage values.", styles["Note"]))

    # 4.3 FDI
    story.append(Paragraph("4.3 FDI Editor (Train Nodes Only)", styles["H2"]))
    story.append(Paragraph(
        "The FDI editor is only enabled for Train nodes. It describes what DCC functions your decoder "
        "supports (headlight, bell, horn, etc.). Each function entry maps a function number (F0-F28) to "
        "a name and behaviour type (binary on/off, or momentary).", styles["Body"]))

    # 4.4 Platform Drivers
    story.append(Paragraph("4.4 Platform Drivers", styles["H2"]))
    story.append(Paragraph(
        "The Platform Drivers section is where you select your target hardware. This is one of the most "
        "important steps \u2014 the platform you choose determines two things: whether the output uses Arduino or "
        "non-Arduino layout, and whether the driver files come pre-filled with working code or just TODO stubs.",
        styles["Body"]))
    story.append(make_table(
        ["Platform", "Framework Layout", "Driver Code"],
        [
            ["ESP32 + TWAI", "Arduino / PlatformIO", "Pre-filled"],
            ["ESP32 + WiFi GridConnect", "Arduino / PlatformIO", "Pre-filled"],
            ["RP2040 + MCP2517FD", "Arduino (Earle Philhower core)", "Pre-filled"],
            ["STM32F4xx + CAN", "STM32 HAL (CubeIDE)", "Pre-filled"],
            ["TI MSPM0 + MCAN", "TI DriverLib (CCS / Theia)", "Pre-filled"],
            ["None / Custom", "Your own toolchain", "TODO stubs"],
        ],
        col_widths=[1.8*inch, 2.2*inch, 1.5*inch]
    ))

    # 4.5 Callbacks
    story.append(Paragraph("4.5 Callbacks", styles["H2"]))
    story.append(Paragraph(
        "Callbacks are the hooks between the library and your application. The Callbacks section lets you "
        "choose which protocol events your application needs to respond to. Each selected callback generates "
        "a stub with a comment explaining what it should do. Unselected callbacks are omitted entirely, "
        "keeping the code clean.", styles["Body"]))

    # 4.6 Generated Files
    story.append(PageBreak())
    story.append(Paragraph("4.6 Generated Files", styles["H2"]))
    story.append(Paragraph(
        "Click the Generated Files tile in the sidebar to preview the output file tree. From there, click "
        "Download ZIP to get your project.", styles["Body"]))
    story.append(make_table(
        ["Generated File", "What It Is"],
        [
            ["openlcb_user_config.h", "Buffer depths and feature flags. Required."],
            ["openlcb_user_config.c", "The node_parameters struct with your node\u2019s metadata."],
            ["main.ino (Arduino)", "Application entry point \u2014 .ino so Arduino IDE recognises it."],
            ["main.c (non-Arduino)", "Application entry point \u2014 plain C for other toolchains."],
            ["application_callbacks/callbacks_*.h", "Callback header stubs."],
            ["callbacks_*.cpp (Arduino)", "Callback implementations \u2014 .cpp so C++ Arduino APIs are available."],
            ["callbacks_*.c (non-Arduino)", "Callback implementations \u2014 plain C."],
            ["application_drivers/openlcb_can_drivers.h", "CAN hardware abstraction header."],
            ["openlcb_can_drivers.cpp / .c", "CAN driver implementation."],
            ["application_drivers/openlcb_drivers.h", "Platform driver header (lock/unlock, config read/write, reboot)."],
            ["openlcb_drivers.cpp / .c", "Platform driver implementation."],
            ["xml_files/cdi.xml", "Your CDI XML definition."],
            ["xml_files/fdi.xml", "Your FDI XML (train nodes only)."],
            ["GETTING_STARTED.txt", "Step-by-step instructions included in every ZIP."],
            ["_project.json", "Saved Wizard state. Reload it to resume editing."],
        ],
        col_widths=[2.2*inch, 4.0*inch]
    ))

    # 4.7 Arduino vs Non-Arduino
    story.append(Paragraph("4.7 Arduino vs. Non-Arduino Differences", styles["H2"]))
    story.append(Paragraph(
        "The platform you select in the Platform Drivers step automatically determines the output layout. "
        "There is no manual checkbox \u2014 the Wizard sets Arduino mode based on the chosen platform. This "
        "changes two things in the generated output:", styles["Body"]))
    story.append(Paragraph(
        "\u2022 <b>Folder structure</b> \u2014 all driver, callback, library, and XML folders move under src/. "
        "Arduino IDE only compiles C/C++ files that are in a subfolder named exactly src/.", styles["Body"]))
    story.append(Paragraph(
        "\u2022 <b>File extensions</b> \u2014 driver and callback source files are generated as .cpp instead of .c. "
        "This lets you call C++ Arduino APIs such as Serial, Wire, and SPI directly inside your driver "
        "and callback implementations.", styles["Body"]))
    story.append(Paragraph(
        "Note: The openlcb_c_lib/ folder is a placeholder \u2014 the Wizard creates the empty folder structure "
        "but does not copy the library. You must copy the openlcb/, drivers/canbus/, and utilities/ source "
        "files there yourself.", styles["Note"]))

    # =========================================================================
    # 5. openlcb_user_config.h
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("5. openlcb_user_config.h in Depth", styles["H1"]))
    story.append(Paragraph(
        "This file is the single most important file in your project. The library uses these constants at "
        "compile time to determine how much memory to allocate. If a value is wrong, the library will either "
        "waste RAM or crash at runtime.", styles["Body"]))

    # 5.1 Feature Flags
    story.append(Paragraph("5.1 Feature Flags", styles["H2"]))
    story.append(code_block(
"""// Uncomment the protocols your node uses.
// Unused protocols are excluded from the build entirely.

#define OPENLCB_COMPILE_EVENTS                // Event producer/consumer
#define OPENLCB_COMPILE_DATAGRAMS             // Required for Config Memory
#define OPENLCB_COMPILE_MEMORY_CONFIGURATION  // CDI and settings via JMRI
// #define OPENLCB_COMPILE_FIRMWARE           // Firmware upgrade support
// #define OPENLCB_COMPILE_STREAM             // Stream transport protocol
// #define OPENLCB_COMPILE_BROADCAST_TIME     // Clock synchronization
// #define OPENLCB_COMPILE_TRAIN             // Locomotive control
// #define OPENLCB_COMPILE_TRAIN_SEARCH      // Throttle train discovery
// #define OPENLCB_COMPILE_DCC_DETECTOR      // DCC detection protocol"""))
    story.append(Paragraph(
        "Note: OPENLCB_COMPILE_MEMORY_CONFIGURATION requires OPENLCB_COMPILE_DATAGRAMS. "
        "OPENLCB_COMPILE_FIRMWARE requires OPENLCB_COMPILE_MEMORY_CONFIGURATION. "
        "OPENLCB_COMPILE_DCC_DETECTOR requires OPENLCB_COMPILE_EVENTS. "
        "OPENLCB_COMPILE_BROADCAST_TIME requires OPENLCB_COMPILE_EVENTS. "
        "OPENLCB_COMPILE_TRAIN_SEARCH requires both OPENLCB_COMPILE_TRAIN and OPENLCB_COMPILE_EVENTS.",
        styles["Note"]))
    story.append(Paragraph(
        "There is also a convenience preset: defining OPENLCB_COMPILE_BOOTLOADER automatically enables "
        "DATAGRAMS, MEMORY_CONFIGURATION, and FIRMWARE.",
        styles["Body"]))

    # 5.2 Buffer Pool
    story.append(Paragraph("5.2 Message Buffer Pool", styles["H2"]))
    story.append(Paragraph(
        "The library uses a pool of fixed-size message buffers. Choose depths that balance RAM use with "
        "the traffic your node will handle.", styles["Body"]))
    story.append(make_table(
        ["Buffer Type (define name)", "Size per Buffer"],
        [
            ["USER_DEFINED_BASIC_BUFFER_DEPTH", "16 bytes each \u2014 most OpenLCB messages fit here"],
            ["USER_DEFINED_DATAGRAM_BUFFER_DEPTH", "72 bytes each \u2014 datagram protocol messages"],
            ["USER_DEFINED_SNIP_BUFFER_DEPTH", "256 bytes each \u2014 SNIP replies and payload events"],
            ["USER_DEFINED_STREAM_BUFFER_DEPTH", "USER_DEFINED_STREAM_BUFFER_LEN bytes each \u2014 stream data transfer"],
        ],
        col_widths=[3.0*inch, 3.2*inch]
    ))

    # 5.3 Node Count
    story.append(Paragraph("5.3 Node Count", styles["H2"]))
    story.append(Paragraph(
        "USER_DEFINED_NODE_BUFFER_DEPTH controls how many virtual nodes this device can host. A simple "
        "accessory decoder uses 1. A command station managing multiple locomotives may use one slot per "
        "active locomotive.", styles["Body"]))

    # 5.4 Event Counts
    story.append(Paragraph("5.4 Event Counts", styles["H2"]))
    story.append(code_block(
"""#define USER_DEFINED_PRODUCER_COUNT       64  // max events this node can produce
#define USER_DEFINED_CONSUMER_COUNT       64  // max events this node can react to
#define USER_DEFINED_PRODUCER_RANGE_COUNT  5  // must be >= 1
#define USER_DEFINED_CONSUMER_RANGE_COUNT  5  // must be >= 1"""))

    # 5.5 Stream Transport
    story.append(Paragraph("5.5 Stream Transport", styles["H2"]))
    story.append(Paragraph(
        "These constants configure the Stream Transport protocol. They are only meaningful when "
        "OPENLCB_COMPILE_STREAM is defined.", styles["Body"]))
    story.append(code_block(
"""// Maximum bytes per stream data frame this node can accept.
// The spec uses a 2-byte field so the protocol max is 65535.
// During negotiation the smaller of the two nodes' buffer sizes wins.
#define USER_DEFINED_STREAM_BUFFER_LEN             256

// How many streams can be open at the same time across all nodes.
// Each active stream uses a small state struct, not a full payload buffer.
#define USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS   1"""))

    # 5.6 Train Protocol
    story.append(Paragraph("5.6 Train Protocol", styles["H2"]))
    story.append(Paragraph(
        "These constants are only meaningful when OPENLCB_COMPILE_TRAIN is defined.", styles["Body"]))
    story.append(code_block(
"""#define USER_DEFINED_TRAIN_NODE_COUNT           4   // max simultaneous train nodes
#define USER_DEFINED_MAX_LISTENERS_PER_TRAIN    6   // max consist members per train
#define USER_DEFINED_MAX_TRAIN_FUNCTIONS       29   // DCC functions: F0 through F28"""))

    # 5.7 Listener Alias Verification
    story.append(Paragraph("5.7 Listener Alias Verification", styles["H2"]))
    story.append(Paragraph(
        "Controls the periodic alias re-verification for consist listener entries. "
        "Only meaningful when OPENLCB_COMPILE_TRAIN is defined.", styles["Body"]))
    story.append(code_block(
"""#define USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL    1    // 100ms ticks between prober calls
#define USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS  250  // 100ms ticks between probes of same entry
#define USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS   30  // 100ms ticks to wait for AMD reply"""))

    # =========================================================================
    # 6. INITIALIZATION
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("6. Initialization \u2014 main.ino", styles["H1"]))
    story.append(Paragraph(
        "The main sketch file wires together the hardware drivers, the library stack, and your application "
        "callbacks. The pattern is always the same four steps: define the config structs, initialize hardware, "
        "initialize the library, create your node(s).", styles["Body"]))

    # 6.1 CAN Config
    story.append(Paragraph("6.1 The CAN Config Struct", styles["H2"]))
    story.append(Paragraph(
        "This struct tells the CAN state machine how to talk to your CAN hardware. All fields are function "
        "pointers. The library calls these when it needs to send a frame, check the TX buffer, or notify you "
        "of incoming frames and alias changes.", styles["Body"]))
    story.append(code_block(
"""static const can_config_t can_config = {
    .transmit_raw_can_frame  = &Esp32CanDriver_transmit_raw_can_frame,
    .is_tx_buffer_clear      = &Esp32CanDriver_is_can_tx_buffer_clear,
    .lock_shared_resources   = &Esp32Drivers_lock_shared_resources,
    .unlock_shared_resources = &Esp32Drivers_unlock_shared_resources,
    .on_rx                   = &CallbacksCan_on_rx,    // optional
    .on_tx                   = &CallbacksCan_on_tx,    // optional
    .on_alias_change         = &CallbacksCan_on_alias_change, // optional
};"""))

    # 6.2 OpenLCB Config
    story.append(Paragraph("6.2 The OpenLCB Config Struct", styles["H2"]))
    story.append(Paragraph(
        "This struct tells the OpenLCB protocol engine how to interact with your application. The most "
        "important entries are the config memory read/write functions and the reboot function. Fields "
        "are conditionally compiled based on your OPENLCB_COMPILE_* flags.", styles["Body"]))
    story.append(code_block(
"""static const openlcb_config_t openlcb_config = {
    // REQUIRED: mutex for thread safety
    .lock_shared_resources   = &Esp32Drivers_lock_shared_resources,
    .unlock_shared_resources = &Esp32Drivers_unlock_shared_resources,

    // Memory Configuration (requires OPENLCB_COMPILE_MEMORY_CONFIGURATION)
    .config_mem_read         = &Esp32Drivers_config_mem_read,
    .config_mem_write        = &Esp32Drivers_config_mem_write,
    .reboot                  = &Esp32Drivers_reboot,       // optional
    .factory_reset           = &CallbacksConfigMem_factory_reset, // optional

    // Firmware Upgrade (requires OPENLCB_COMPILE_FIRMWARE)
    .freeze                  = &CallbacksConfigMem_freeze,
    .unfreeze                = &CallbacksConfigMem_unfreeze,
    .firmware_write          = &CallbacksConfigMem_firmware_write,

    // Application callbacks
    .on_100ms_timer          = &CallbacksOlcb_on_100ms_timer,
    .on_consumed_event_pcer  = &CallbacksOlcb_on_event, // optional (EVENTS)
};"""))
    story.append(Paragraph(
        "The struct contains additional optional callback fields for event identification, broadcast time, "
        "train control, train search, and stream transport. Set any field to NULL (or omit it) to use the "
        "library default. See openlcb_config.h for the full list.", styles["Note"]))

    # 6.3 Setup and Loop
    story.append(Paragraph("6.3 Setup and Loop", styles["H2"]))
    story.append(code_block(
"""void setup() {
    Serial.begin(9600);

    // 1. Initialize hardware
    Esp32CanDriver_initialize();
    Esp32Drivers_initialize();

    // 2. Initialize the library stacks
    CanConfig_initialize(&can_config);
    OpenLcbConfig_initialize(&openlcb_config);

    // 3. Initialize your application callbacks
    CallbacksOlcb_initialize();

    // 4. Create the node
    OpenLcbConfig_create_node(NODE_ID, &OpenLcbUserConfig_node_parameters);
}

void loop() {
    // Run all state machines. Call as fast as possible.
    OpenLcbConfig_run();

    // Your application code here. Keep it short and non-blocking.
}"""))
    story.append(Paragraph(
        "Note: OpenLcbConfig_run() must be called repeatedly from your main loop. Do not use delay() in "
        "loop() \u2014 it stalls the state machines.", styles["Note"]))

    # =========================================================================
    # 7. DRIVERS
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("7. Implementing the Drivers", styles["H1"]))
    story.append(Paragraph(
        "The two driver files are the only hardware-specific code in a project. If you are using the "
        "ESP32 BasicNode example, these are already implemented. This section explains what each function "
        "must do when porting to a new platform.", styles["Body"]))

    story.append(Paragraph("7.1 CAN Bus Drivers (esp32_can_drivers.cpp)", styles["H2"]))
    story.append(Paragraph("transmit_raw_can_frame", styles["H3"]))
    story.append(Paragraph(
        "Called by the library to send one CAN frame. The library will not call this again until "
        "is_tx_buffer_clear() returns true.", styles["Body"]))
    story.append(code_block(
"""bool Esp32CanDriver_transmit_raw_can_frame(can_msg_t *can_msg) {
    twai_message_t msg;
    msg.extd = 1;  // OpenLCB uses 29-bit extended IDs
    msg.identifier = can_msg->id;
    msg.data_length_code = can_msg->dlc;
    for (int i = 0; i < can_msg->dlc; i++)
        msg.data[i] = can_msg->data[i];
    return (twai_transmit(&msg, 0) == ESP_OK);  // non-blocking
}"""))
    story.append(Paragraph("is_tx_buffer_clear", styles["H3"]))
    story.append(code_block(
"""bool Esp32CanDriver_is_can_tx_buffer_clear(void) {
    twai_status_info_t info;
    twai_get_status_info(&info);
    return (info.msgs_to_tx == 0);
}"""))
    story.append(Paragraph("Receiving Frames", styles["H3"]))
    story.append(Paragraph(
        "Incoming CAN frames are not pulled by the library. Your setup code creates a FreeRTOS task (or "
        "polling loop) that reads frames from the hardware and passes them into the library:", styles["Body"]))
    story.append(code_block(
"""void can_receive_task(void *param) {
    twai_message_t msg;
    for (;;) {
        if (twai_receive(&msg, pdMS_TO_TICKS(10)) == ESP_OK) {
            can_msg_t can_msg;
            can_msg.id  = msg.identifier;
            can_msg.dlc = msg.data_length_code;
            for (int i = 0; i < msg.data_length_code; i++)
                can_msg.data[i] = msg.data[i];
            CanMainStateMachine_process_rx_frame(&can_msg);
        }
    }
}"""))

    # 7.2 Platform Drivers
    story.append(Paragraph("7.2 OpenLCB Platform Drivers (esp32_drivers.cpp)", styles["H2"]))
    story.append(Paragraph("Configuration Memory Read and Write", styles["H3"]))
    story.append(Paragraph(
        "The library calls these when JMRI reads or writes your node\u2019s settings. You map addresses to your "
        "NVS or EEPROM.", styles["Body"]))
    story.append(code_block(
"""uint16_t Esp32Drivers_config_mem_read(openlcb_node_t *node,
    uint32_t address, uint16_t count,
    configuration_memory_buffer_t *buffer)
{
    preferences.getBytes("cfg", scratch_buf, sizeof(scratch_buf));
    memcpy(buffer, scratch_buf + address, count);
    return count;
}

uint16_t Esp32Drivers_config_mem_write(openlcb_node_t *node,
    uint32_t address, uint16_t count,
    configuration_memory_buffer_t *buffer)
{
    preferences.getBytes("cfg", scratch_buf, sizeof(scratch_buf));
    memcpy(scratch_buf + address, buffer, count);
    preferences.putBytes("cfg", scratch_buf, sizeof(scratch_buf));
    return count;
}"""))

    # =========================================================================
    # 8. CALLBACKS
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("8. Callbacks \u2014 Where Your Application Lives", styles["H1"]))
    story.append(Paragraph(
        "Callbacks are function stubs the library calls when protocol events occur. This is where you write "
        "your application logic. The Node Wizard generates the function signatures with TODO comments. You "
        "fill in the bodies.", styles["Body"]))

    story.append(Paragraph("CallbacksOlcb_initialize()", styles["H3"]))
    story.append(Paragraph(
        "Called once during setup, after OpenLcbConfig_initialize(). Use this to register event IDs, load "
        "settings from NVS, and initialize output pins.", styles["Body"]))
    story.append(code_block(
"""void CallbacksOlcb_initialize(void) {
    openlcb_node_t *node = OpenLcbNode_get_first(0);

    // Register events this node produces
    OpenLcbApplication_register_producer_eventid(
        node, 0x0501010107770001, EVENT_STATUS_CLEAR);

    // Register events this node consumes
    OpenLcbApplication_register_consumer_eventid(
        node, 0x0501010107770002, EVENT_STATUS_UNKNOWN);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
}"""))

    story.append(Paragraph("CallbacksOlcb_on_100ms_timer()", styles["H3"]))
    story.append(Paragraph(
        "The library calls this function every 100 milliseconds. Good place for polling inputs and "
        "debouncing buttons.", styles["Body"]))
    story.append(code_block(
"""static uint8_t button_prev = HIGH;

void CallbacksOlcb_on_100ms_timer(void) {
    openlcb_node_t *node = OpenLcbNode_get_first(0);
    uint8_t button_now = digitalRead(BUTTON_PIN);
    if (button_now == LOW && button_prev == HIGH)
        OpenLcbApplication_send_event_pc_report(
            node, 0x0501010107770001);
    button_prev = button_now;
}"""))

    story.append(Paragraph("CAN Observation Callbacks (Optional)", styles["H3"]))
    story.append(Paragraph(
        "CallbacksCan_on_rx and CallbacksCan_on_tx let you observe every raw CAN frame. Most "
        "applications leave these as empty stubs. Useful for debugging.", styles["Body"]))

    # =========================================================================
    # 9. EVENTS IN DEPTH
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("9. Events in Depth", styles["H1"]))
    story.append(Paragraph("Choosing Event IDs", styles["H2"]))
    story.append(Paragraph(
        "An Event ID is a 64-bit number. The convention is: the upper 6 bytes come from your Node ID, and "
        "the lower 2 bytes are a sequence number you choose. This ensures your event IDs are globally unique.",
        styles["Body"]))
    story.append(code_block(
"""// Pattern: Node ID bytes + your own sequence number
// Node ID: 0x05.01.01.01.07.77
#define EVENT_BUTTON_PRESSED   0x0501010107770001
#define EVENT_BUTTON_RELEASED  0x0501010107770002
#define EVENT_SENSOR_ACTIVE    0x0501010107770003
#define EVENT_SENSOR_INACTIVE  0x0501010107770004"""))

    story.append(Paragraph("Event Status Values", styles["H2"]))
    story.append(make_table(
        ["Status", "Meaning"],
        [
            ["EVENT_STATUS_CLEAR", "The condition this event represents is currently inactive/off."],
            ["EVENT_STATUS_SET", "The condition is currently active/on."],
            ["EVENT_STATUS_UNKNOWN", "Current state is unknown (most common for consumers at startup)."],
        ],
        col_widths=[2.0*inch, 4.2*inch]
    ))

    story.append(Paragraph(
        "Note: Event IDs in configuration memory are big-endian (most significant byte first). On "
        "little-endian processors like ESP32, you may need to byte-swap them after reading.", styles["Note"]))

    # =========================================================================
    # 10. CONFIGURATION MEMORY
    # =========================================================================
    story.append(Paragraph("10. Configuration Memory", styles["H1"]))
    story.append(Paragraph(
        "When OPENLCB_COMPILE_MEMORY_CONFIGURATION is enabled, configuration tools like JMRI can read and "
        "write your node\u2019s settings remotely. The settings are stored as a flat byte array in non-volatile "
        "storage. The CDI tells JMRI how to interpret the bytes.", styles["Body"]))
    story.append(Paragraph("Memory Space Layout", styles["H2"]))
    story.append(make_table(
        ["Space", "Purpose"],
        [
            ["253 (0xFD)", "User configuration. CDI describes this space. JMRI reads and writes here."],
            ["252 (0xFC)", "All memory (read only). Used internally by the library."],
            ["251 (0xFB)", "CDI XML. The library serves CDI from here."],
            ["249 (0xF9)", "FDI XML (train nodes only)."],
        ],
        col_widths=[1.2*inch, 5.0*inch]
    ))

    # =========================================================================
    # 11. TRAIN NODES
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("11. Train Nodes (Brief Overview)", styles["H1"]))
    story.append(Paragraph(
        "Select Train or Train Controller in the Node Wizard to add train-specific protocols.", styles["Body"]))
    story.append(Paragraph("Train Node", styles["H2"]))
    story.append(Paragraph(
        "A Train node represents a single locomotive and accepts speed and function commands from throttles "
        "over the LCC network. The openlcb_config_t struct contains callback fields for speed changes, "
        "function changes, emergency stop, controller assignment, and listener management. All are optional "
        "\u2014 set to NULL to use library defaults.", styles["Body"]))
    story.append(Paragraph("Train Controller", styles["H2"]))
    story.append(Paragraph(
        "A Train Controller (throttle or command station) discovers and commands trains on the network using "
        "the Train Search protocol. This node type does not have an FDI.", styles["Body"]))

    # =========================================================================
    # 12. PLATFORMIO
    # =========================================================================
    story.append(Paragraph("12. Using PlatformIO Instead of Arduino IDE", styles["H1"]))
    story.append(Paragraph(
        "PlatformIO in VSCode is an alternative to Arduino IDE. Here is a minimal platformio.ini for the "
        "BasicNode example:", styles["Body"]))
    story.append(code_block(
"""[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

; Add the sketch directory to the include path
build_flags = -I src"""))
    story.append(Paragraph(
        "Place all source files in the src/ directory. PlatformIO will compile all .c and .cpp files in "
        "src/ automatically.", styles["Body"]))

    # =========================================================================
    # 13. TROUBLESHOOTING
    # =========================================================================
    story.append(Paragraph("13. Troubleshooting", styles["H1"]))
    story.append(make_table(
        ["Symptom", "Likely Cause and Fix"],
        [
            ["Node does not appear in JMRI",
             "Check CAN wiring (TX/RX may be swapped). Verify 120-ohm termination resistors on bus. Check NODE_ID is unique."],
            ["Undefined reference to USER_DEFINED_*",
             "openlcb_user_config.h is not on the include path. For Arduino IDE it must be in the sketch folder. For PlatformIO add -I src."],
            ["Node appears then immediately disappears",
             "Another node has the same Node ID. Every node must have a unique ID."],
            ["JMRI config panel is empty",
             "CDI is malformed or the CDI byte array was not regenerated after editing the XML. Re-run the cdi_to_array tool."],
            ["Settings do not save across power cycles",
             "config_mem_write is not writing to NVS. Add Serial.println() in the write function to confirm it is being called."],
            ["Events not received by consumer",
             "Verify both nodes are on the same bus. Verify the consumer registered the exact same 64-bit event ID as the producer."],
            ["Buffer depth compile error on 8-bit processors",
             "Total buffer count (sum of all four depth defines) must not exceed 126 on 8-bit platforms."],
        ],
        col_widths=[2.0*inch, 4.2*inch]
    ))

    story.append(Paragraph("Enabling Verbose Compilation Output", styles["H2"]))
    story.append(code_block(
"""// In openlcb_user_config.h:
#define OPENLCB_COMPILE_VERBOSE

// Prints a feature summary confirming which protocols were compiled in."""))

    story.append(Spacer(1, 12))
    story.append(Paragraph("Online Resources", styles["H2"]))
    story.append(Paragraph(
        "Library documentation: https://jimkueneman.github.io/OpenLcbCLib/", styles["Body"]))
    story.append(Paragraph(
        "OpenLCB/LCC standards: https://www.nmra.org/lcc", styles["Body"]))


PAGE_NUM = [0]

def footer(canvas_obj, doc):
    PAGE_NUM[0] += 1
    canvas_obj.saveState()
    canvas_obj.setFont("Helvetica", 7.5)
    canvas_obj.setFillColor(MED_GRAY)
    w = letter[0]
    canvas_obj.drawString(0.75*inch, 0.4*inch,
        f"OpenLcbCLib | Developer Guide")
    canvas_obj.drawRightString(w - 0.75*inch, 0.4*inch,
        f"Page {PAGE_NUM[0]}")
    canvas_obj.restoreState()


def main():
    doc = SimpleDocTemplate(
        OUTPUT, pagesize=letter,
        topMargin=0.6*inch, bottomMargin=0.65*inch,
        leftMargin=0.75*inch, rightMargin=0.75*inch,
        title="OpenLcbCLib Developer Guide",
        author="Jim Kueneman",
    )
    story = []
    build(story)
    doc.build(story, onFirstPage=footer, onLaterPages=footer)
    print(f"Created {OUTPUT}")


if __name__ == "__main__":
    main()
