#!/usr/bin/env python3
"""Generate self-signed SSL certificate for RPAL IDE"""

import subprocess
import sys
from pathlib import Path

CERT_DIR = Path(__file__).parent / ".certs"
CERT_FILE = CERT_DIR / "rpal_cert.pem"
KEY_FILE = CERT_DIR / "rpal_key.pem"


def generate_certificate():
    """Generate self-signed certificate using OpenSSL"""
    
    # Check if OpenSSL is available
    try:
        subprocess.run(["openssl", "version"], capture_output=True, check=True)
    except FileNotFoundError:
        print("❌ OpenSSL not found. Install it to generate certificates.")
        print("   Windows: https://slproweb.com/products/Win32OpenSSL.html")
        print("   macOS: brew install openssl")
        print("   Linux: apt-get install openssl")
        sys.exit(1)

    CERT_DIR.mkdir(exist_ok=True)

    print("🔐 Generating self-signed SSL certificate...")
    print(f"   Cert: {CERT_FILE}")
    print(f"   Key: {KEY_FILE}")

    try:
        subprocess.run(
            [
                "openssl", "req", "-x509", "-newkey", "rsa:4096",
                "-keyout", str(KEY_FILE), "-out", str(CERT_FILE),
                "-days", "365", "-nodes",
                "-subj", "/C=US/ST=State/L=City/O=Org/CN=localhost"
            ],
            check=True,
            capture_output=True
        )

        # Set proper permissions (Unix only)
        try:
            KEY_FILE.chmod(0o600)
            CERT_FILE.chmod(0o644)
        except (OSError, AttributeError):
            pass

        print("✅ Certificate generated successfully!")
        print("\n📝 To use in production:")
        print("   1. Replace with your CA-signed certificate")
        print("   2. Set environment variables:")
        print(f"      RPAL_USE_SSL=true")
        print(f"      RPAL_SSL_CERT={CERT_FILE}")
        print(f"      RPAL_SSL_KEY={KEY_FILE}")
        print("   3. Access via https://yourdomain.com/")

    except subprocess.CalledProcessError as e:
        print(f"❌ Error generating certificate: {e.stderr.decode()}")
        sys.exit(1)


if __name__ == "__main__":
    generate_certificate()
