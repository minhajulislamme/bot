#!/bin/bash

echo "=== Detailed WebSocket Connection Test ==="
echo "Testing connection to Binance Futures WebSocket server..."

# Test basic connectivity first
echo "1. Testing basic network connectivity to stream.binancefuture.com..."
ping -c 3 stream.binancefuture.com
if [ $? -ne 0 ]; then
    echo "❌ Failed to ping WebSocket server. Check DNS and network connectivity."
    exit 1
fi

# Test SSL handshake
echo "2. Testing SSL handshake with WebSocket server..."
openssl s_client -connect stream.binancefuture.com:443 </dev/null 2>/dev/null | grep "Verify return code"
if [ $? -ne 0 ]; then
    echo "❌ Failed to establish SSL connection to WebSocket server."
    echo "Check if SSL/TLS connections are allowed by your firewall."
else
    echo "✅ SSL handshake successful"
fi

# Test WebSocket protocol upgrade
echo "3. Testing WebSocket protocol upgrade..."
timeout 10 curl -v --include --no-buffer --header "Connection: Upgrade" \
    --header "Upgrade: websocket" \
    --header "Host: stream.binancefuture.com" \
    --header "Origin: https://stream.binancefuture.com" \
    --header "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==" \
    --header "Sec-WebSocket-Version: 13" \
    -k https://stream.binancefuture.com/ws 2>&1 | grep -i "101 Switching Protocols"

if [ $? -eq 0 ]; then
    echo "✅ WebSocket protocol upgrade successful"
else
    echo "❌ WebSocket protocol upgrade failed"
    echo "Details:"
    timeout 10 curl -v --include --no-buffer --header "Connection: Upgrade" \
        --header "Upgrade: websocket" \
        --header "Host: stream.binancefuture.com" \
        --header "Origin: https://stream.binancefuture.com" \
        --header "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==" \
        --header "Sec-WebSocket-Version: 13" \
        -k https://stream.binancefuture.com/ws 2>&1 | head -20
fi

echo "4. Testing firewall rules..."
echo "Checking if outbound connections to port 443 are allowed..."
nc -zv stream.binancefuture.com 443 -w 5
if [ $? -eq 0 ]; then
    echo "✅ Port 443 is open for outbound connections"
else
    echo "❌ Cannot establish connection to port 443"
    echo "Please check if your firewall allows outbound connections on port 443"
fi

echo ""
echo "=== Recommendations ==="
echo "If WebSocket connectivity is failing, try the following:"
echo "1. Check if your server allows outbound WebSocket connections (port 443)"
echo "2. Verify your network's proxy settings if applicable"
echo "3. Make sure your server's SSL/TLS certificates are up to date"
echo "4. Try connecting to the alternate WebSocket endpoint: fstream.binancefuture.com"
