#!/bin/bash
# RPAL IDE - Production Deployment Quick Start

set -e

echo "🚀 RPAL IDE - SSL/HTTPS Setup"
echo "=============================="
echo ""

# Check prerequisites
echo "📋 Checking prerequisites..."
command -v openssl >/dev/null 2>&1 || { echo "❌ openssl not found"; exit 1; }
command -v docker >/dev/null 2>&1 || { echo "⚠️  docker not found (optional for local dev)"; }
command -v python3 >/dev/null 2>&1 || { echo "❌ python3 not found"; exit 1; }

echo "✅ Prerequisites OK"
echo ""

# Ask user for setup type
echo "Choose setup type:"
echo "1) Self-Signed Certificate (Development/Internal)"
echo "2) Let's Encrypt (Production Linux with Nginx)"
echo "3) Docker with Nginx (Cloud)"
read -p "Enter choice (1-3): " choice

case $choice in
    1)
        echo ""
        echo "🔐 Setting up Self-Signed Certificate..."
        cd web/server
        python3 generate_cert.py
        echo ""
        echo "✅ Ready! Run with:"
        echo "   export RPAL_USE_SSL=true"
        echo "   export RPAL_SSL_CERT=web/server/.certs/rpal_cert.pem"
        echo "   export RPAL_SSL_KEY=web/server/.certs/rpal_key.pem"
        echo "   python3 app.py"
        echo ""
        echo "   Then visit: https://127.0.0.1:8787/"
        ;;
    2)
        echo ""
        echo "🔒 Setting up Let's Encrypt (Production)..."
        echo "This requires:"
        echo "  • Linux server (Ubuntu/Debian)"
        echo "  • Domain name pointing to your server"
        echo "  • Port 80 and 443 open"
        echo ""
        echo "Follow: docs/HTTPS_SETUP.md - Option 2"
        ;;
    3)
        echo ""
        echo "🐳 Setting up Docker with Nginx..."
        echo ""
        read -p "Enter your domain (e.g., rpal-ide.example.com): " domain
        
        # Update nginx config with domain
        sed -i "s/rpal-ide.yourdomain.com/$domain/g" web/nginx.conf
        
        echo "✅ Domain configured: $domain"
        echo ""
        echo "Ready to deploy! Run:"
        echo "   docker-compose up -d"
        echo ""
        echo "Get SSL certificate:"
        echo "   docker-compose run certbot"
        ;;
    *)
        echo "❌ Invalid choice"
        exit 1
        ;;
esac

echo ""
echo "📚 For detailed setup: see HTTPS_SETUP.md"
echo "🔗 For CI/CD: see CICD_SETUP.md"
