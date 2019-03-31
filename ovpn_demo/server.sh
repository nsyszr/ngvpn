#!/bin/sh
openvpn --config ./server.conf --mode server --script-security 3 --plugin ../src/ngvpn-plugin/ngvpn-plugin.so
