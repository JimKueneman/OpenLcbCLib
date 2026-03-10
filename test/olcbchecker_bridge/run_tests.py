#!/usr/bin/env python3
"""
Custom OlcbChecker test runner with section selection.

Calls each test section's checkAll() directly, bypassing OlcbChecker's
interactive menu system.  Section selection is controlled via environment
variables set by run_olcbchecker.sh:

  Multi-pass mode (ComplianceTestNode):
    RUN_SECTIONS="core"                   Run core tests only
    RUN_SECTIONS="broadcast_time_consumer"  Run BT consumer tests only
    RUN_SECTIONS="trains"                 Run train tests only

  Legacy mode (BasicNode):
    RUN_BROADCAST_TIME=1   Include Broadcast Time consumer tests
    RUN_TRAINS=1           Include Train Control / Search / FDI tests

If RUN_SECTIONS is set, it takes precedence. Otherwise falls back to the
legacy RUN_BROADCAST_TIME / RUN_TRAINS flags.

All standard OlcbChecker CLI flags (-a, -t, -i, --auto-reboot, etc.) are
passed through via sys.argv.
"""

import sys
import os
import logging

OLCBCHECKER_DIR = "/Users/jimkueneman/Documents/OlcbChecker"
sys.path.insert(0, OLCBCHECKER_DIR)
os.chdir(OLCBCHECKER_DIR)

# Determine which sections to run
run_sections_env = os.environ.get("RUN_SECTIONS", "")
single_script = os.environ.get("SINGLE_SCRIPT", "")

if run_sections_env:
    # Multi-pass mode: specific section(s) requested
    run_sections = set(run_sections_env.split(","))
else:
    # Legacy mode: core always runs, optional sections via flags
    run_sections = {"core"}
    if os.environ.get("RUN_BROADCAST_TIME", "0") == "1":
        run_sections.add("broadcast_time_consumer")
    if os.environ.get("RUN_TRAINS", "0") == "1":
        run_sections.add("trains")

# OlcbChecker's configure.py parses sys.argv on import;
# olcbchecker.setup then opens the TCP/serial connection.
import olcbchecker.setup

logger = logging.getLogger("OLCBCHECKER")
total = 0

# ---- Single-script mode ---------------------------------------------------

if "single" in run_sections and single_script:
    import importlib
    logger.info("=== Single: {} ===".format(single_script))
    try:
        mod = importlib.import_module(single_script)
    except ModuleNotFoundError:
        logger.error("Module '{}' not found in {}".format(single_script, OLCBCHECKER_DIR))
        olcbchecker.setup.interface.close()
        sys.exit(1)

    # control_* modules have checkAll(), check_* modules have check()
    if hasattr(mod, "checkAll"):
        total += min(mod.checkAll(), 1)
    elif hasattr(mod, "check"):
        result = mod.check()
        total += min(result, 1)
        if result == 0:
            logger.info("PASSED")
        else:
            logger.warning("FAILED (result={})".format(result))
    else:
        logger.error("Module '{}' has neither check() nor checkAll()".format(single_script))
        olcbchecker.setup.interface.close()
        sys.exit(1)

    olcbchecker.setup.interface.close()
    sys.exit(total)

# ---- Core protocol tests --------------------------------------------------

if "core" in run_sections:
    import control_frame
    logger.info("=== Frame Transport ===")
    total += min(control_frame.checkAll(), 1)

    import control_message
    logger.info("=== Message Network ===")
    total += min(control_message.checkAll(), 1)

    import control_snip
    logger.info("=== SNIP ===")
    total += min(control_snip.checkAll(), 1)

    import control_events
    logger.info("=== Event Transport ===")
    total += min(control_events.checkAll(), 1)

    import control_datagram
    logger.info("=== Datagram Transport ===")
    total += min(control_datagram.checkAll(), 1)

    import control_memory
    logger.info("=== Memory Configuration ===")
    total += min(control_memory.checkAll(), 1)

    import control_cdi
    logger.info("=== CDI ===")
    total += min(control_cdi.checkAll(), 1)

# ---- Broadcast Time Consumer ----------------------------------------------

if "broadcast_time_consumer" in run_sections:
    import control_broadcasttime
    logger.info("=== Broadcast Time (Consumer) ===")
    total += min(control_broadcasttime.checkAllConsumer(), 1)

# ---- Broadcast Time Producer ----------------------------------------------

if "broadcast_time_producer" in run_sections:
    import control_broadcasttime
    logger.info("=== Broadcast Time (Producer) ===")
    total += min(control_broadcasttime.checkAllProducer(), 1)

# ---- Train Protocols -------------------------------------------------------

if "trains" in run_sections:
    import control_traincontrol
    logger.info("=== Train Control ===")
    total += min(control_traincontrol.checkAll(), 1)

    import control_trainsearch
    logger.info("=== Train Search ===")
    total += min(control_trainsearch.checkAll(), 1)

    import control_fdi
    logger.info("=== FDI ===")
    total += min(control_fdi.checkAll(), 1)

# ---- Add new protocol sections here ----------------------------------------
# if "traction_proxy" in run_sections:
#     import control_tractionproxy
#     total += min(control_tractionproxy.checkAll(), 1)

# ---- Summary ---------------------------------------------------------------

if total > 0:
    logger.info("{} sections had failures".format(total))
else:
    logger.info("All sections passed")

olcbchecker.setup.interface.close()
sys.exit(total)
