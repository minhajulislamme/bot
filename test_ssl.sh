#!/bin/bash

echo "Testing SSL capabilities..."
echo "OpenSSL version:"
openssl version

echo "Checking CA certificates..."
if [ -f /etc/ssl/certs/ca-certificates.crt ]; then
  echo "CA certificates found at /etc/ssl/certs/ca-certificates.crt"
else
  echo "CA certificates not found at /etc/ssl/certs/ca-certificates.crt"
  if [ -f /etc/pki/tls/certs/ca-bundle.crt ]; then
    echo "CA certificates found at /etc/pki/tls/certs/ca-bundle.crt"
  else
    echo "CA certificates not found at /etc/pki/tls/certs/ca-bundle.crt"
    echo "WARNING: No standard CA certificate bundles found"
  fi
fi

echo "Testing SSL connection to Binance WebSocket server..."
openssl s_client -connect stream.binancefuture.com:443 < /dev/null | grep "Verify return code"

echo "Done."
