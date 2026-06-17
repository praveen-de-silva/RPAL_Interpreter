"""RPAL IDE Server Configuration"""
import os
import ssl
from pathlib import Path

# Environment
ENV = os.getenv("RPAL_ENV", "development")  # development | staging | production
DEBUG = ENV == "development"

# Server Settings
HOST = os.getenv("RPAL_IDE_HOST", "127.0.0.1" if DEBUG else "0.0.0.0")
PORT = int(os.getenv("RPAL_IDE_PORT", "8787"))

# SSL/TLS Settings
USE_SSL = os.getenv("RPAL_USE_SSL", "false").lower() == "true"
SSL_CERT_FILE = os.getenv("RPAL_SSL_CERT", "/etc/ssl/certs/rpal_cert.pem")
SSL_KEY_FILE = os.getenv("RPAL_SSL_KEY", "/etc/ssl/private/rpal_key.pem")

# Security Headers
SECURITY_HEADERS = {
    "X-Content-Type-Options": "nosniff",
    "X-Frame-Options": "DENY",
    "X-XSS-Protection": "1; mode=block",
    "Strict-Transport-Security": "max-age=31536000; includeSubDomains" if ENV == "production" else "",
    "Content-Security-Policy": "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'",
}

# CORS Settings
ALLOWED_ORIGINS = os.getenv("RPAL_ALLOWED_ORIGINS", "localhost").split(",")

# Interpreter Settings
TIMEOUT_SECONDS = int(os.getenv("RPAL_TIMEOUT", "30"))
MAX_CODE_SIZE_KB = int(os.getenv("RPAL_MAX_CODE_SIZE", "100"))


def get_ssl_context() -> ssl.SSLContext | None:
    """Create SSL context for HTTPS"""
    if not USE_SSL:
        return None

    cert_path = Path(SSL_CERT_FILE)
    key_path = Path(SSL_KEY_FILE)

    if not cert_path.is_file() or not key_path.is_file():
        raise FileNotFoundError(
            f"SSL certificate not found. Expected:\n"
            f"  Cert: {cert_path}\n"
            f"  Key: {key_path}\n"
            f"Run: python generate_cert.py"
        )

    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(certfile=str(cert_path), keyfile=str(key_path))
    context.options |= ssl.OP_NO_TLSv1 | ssl.OP_NO_TLSv1_1  # Disable old protocols
    return context
