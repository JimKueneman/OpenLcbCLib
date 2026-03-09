#!/usr/bin/env python3
"""
TCP Bridge Server for OlcbChecker Integration Testing

Accepts two TCP client connections on a single port and forwards GridConnect
strings bidirectionally between them:

  Client 1 (BasicNode)  <-->  Bridge  <-->  Client 2 (OlcbChecker)

Both the BasicNode C binary and OlcbChecker are TCP clients. This bridge
server sits in the middle so they can communicate.

Usage:
    python3 bridge_server.py [--port PORT] [--verbose]
"""

import socket
import threading
import sys
import argparse
import signal

# ============================================================================
# Configuration
# ============================================================================

OLCBCHECKER_DIR = "/Users/jimkueneman/Documents/OlcbChecker"
DEFAULT_PORT = 12021

# ============================================================================
# Bridge Implementation
# ============================================================================

shutdown_event = threading.Event()


def forward(src, dst, label, verbose=False):
    """Read from src socket, write to dst socket, forwarding all data."""
    try:
        while not shutdown_event.is_set():
            data = src.recv(4096)
            if not data:
                break
            if verbose:
                lines = data.decode('ascii', errors='replace').strip().split('\n')
                for line in lines:
                    line = line.strip()
                    if line:
                        print(f"  [{label}] {line}")
            dst.sendall(data)
    except (ConnectionResetError, BrokenPipeError, OSError):
        pass
    finally:
        if verbose:
            print(f"  [{label}] connection closed")


def run_bridge(port=DEFAULT_PORT, verbose=False):
    """Start the bridge server and accept client pairs in a loop.

    Each pass of the test runner connects two clients (node + checker).
    When either disconnects, the bridge cleans up and waits for the next pair.
    This continues until the process receives SIGTERM/SIGINT.
    """

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.settimeout(1.0)
    server.bind(('127.0.0.1', port))
    server.listen(2)

    print(f"Bridge: listening on 127.0.0.1:{port}")

    def accept_client(name):
        """Accept a client connection, respecting shutdown."""
        while not shutdown_event.is_set():
            try:
                conn, addr = server.accept()
                print(f"Bridge: {name} connected from {addr}")
                return conn
            except socket.timeout:
                continue
        return None

    while not shutdown_event.is_set():
        # Wait for node to connect (first client)
        node_conn = accept_client("Node")
        if node_conn is None:
            break

        # Wait for checker to connect (second client)
        checker_conn = accept_client("OlcbChecker")
        if checker_conn is None:
            node_conn.close()
            break

        print("Bridge: both clients connected, forwarding traffic")

        # Forward bidirectionally
        t1 = threading.Thread(
            target=forward,
            args=(node_conn, checker_conn, "Node->Checker", verbose),
            daemon=True
        )
        t2 = threading.Thread(
            target=forward,
            args=(checker_conn, node_conn, "Checker->Node", verbose),
            daemon=True
        )
        t1.start()
        t2.start()

        # Wait for either direction to finish (means one side disconnected)
        t1.join()
        t2.join()

        print("Bridge: pass complete, waiting for next pair")
        node_conn.close()
        checker_conn.close()

    print("Bridge: shutting down")
    server.close()
    return 0


def main():
    parser = argparse.ArgumentParser(
        description="TCP bridge server for OlcbChecker integration testing"
    )
    parser.add_argument(
        '--port', type=int, default=DEFAULT_PORT,
        help=f"Port to listen on (default: {DEFAULT_PORT})"
    )
    parser.add_argument(
        '--verbose', '-v', action='store_true',
        help="Print all GridConnect traffic passing through the bridge"
    )
    args = parser.parse_args()

    # Handle Ctrl-C and SIGTERM gracefully
    def signal_handler(signum, frame):
        print("\nBridge: received signal, shutting down...")
        shutdown_event.set()

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    return run_bridge(port=args.port, verbose=args.verbose)


if __name__ == '__main__':
    sys.exit(main())
