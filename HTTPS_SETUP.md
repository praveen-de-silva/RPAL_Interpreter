# SSL/HTTPS Setup Guide for RPAL IDE

## 🔐 Three Ways to Setup HTTPS

### Option 1: Self-Signed Certificate (Development)

**Perfect for:** Local testing, staging, internal deployments

```bash
cd web/server

# Generate self-signed certificate
python generate_cert.py

# Certificate created in: .certs/
# - rpal_cert.pem (public)
# - rpal_key.pem (private)
```

**Run with SSL:**
```bash
export RPAL_USE_SSL=true
export RPAL_SSL_CERT=web/server/.certs/rpal_cert.pem
export RPAL_SSL_KEY=web/server/.certs/rpal_key.pem
python web/server/app.py
```

**Access:** `https://127.0.0.1:8787/` (ignore browser warning)

---

### Option 2: Let's Encrypt Free Certificate (Production Linux)

**Perfect for:** Public-facing production deployments on Linux

#### Prerequisites
```bash
sudo apt update
sudo apt install certbot certbot-nginx python3-pip
pip install python-dotenv
```

#### Setup

1. **Get certificate:**
```bash
sudo certbot certonly --nginx -d rpal-ide.yourdomain.com
```

2. **Configure Nginx** - Copy content from `web/nginx.conf`:
```bash
sudo nano /etc/nginx/sites-available/rpal-ide
# Paste the nginx configuration
# Edit: rpal-ide.yourdomain.com to your domain
```

3. **Enable site:**
```bash
sudo ln -s /etc/nginx/sites-available/rpal-ide /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl restart nginx
```

4. **Auto-renewal:**
```bash
sudo systemctl enable certbot.timer
sudo systemctl start certbot.timer
```

5. **Run Python app:**
```bash
export RPAL_IDE_HOST=127.0.0.1  # Only localhost
export RPAL_IDE_PORT=8787
python web/server/app.py
```

**Access:** `https://rpal-ide.yourdomain.com/`

---

### Option 3: Docker with Nginx + SSL

**Perfect for:** Cloud deployments (AWS, Azure, DigitalOcean)

#### Docker Setup

1. **Update Dockerfile** for production:
```dockerfile
FROM python:3.11-slim

WORKDIR /app

COPY compiler/bin/rpal20 /app/compiler/bin/rpal20
COPY web/server/ /app/server/
COPY web/public/ /app/public/

WORKDIR /app/server
RUN pip install --no-cache-dir -r requirements.txt

EXPOSE 8787
ENV RPAL_IDE_PORT=8787
ENV RPAL_IDE_HOST=127.0.0.1
CMD ["python", "app.py"]
```

2. **Docker Compose with Nginx + Certbot:**
```yaml
version: '3.8'

services:
  rpal-app:
    build: .
    ports:
      - "127.0.0.1:8787:8787"
    environment:
      RPAL_IDE_HOST: 127.0.0.1
      RPAL_IDE_PORT: 8787
    volumes:
      - ./compiler/bin:/app/compiler/bin:ro
    restart: unless-stopped
    networks:
      - rpal-net

  nginx:
    image: nginx:alpine
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./web/nginx.conf:/etc/nginx/conf.d/default.conf:ro
      - ./certs:/etc/nginx/certs:ro
      - ./certbot:/var/www/certbot:ro
    depends_on:
      - rpal-app
    restart: unless-stopped
    networks:
      - rpal-net

  certbot:
    image: certbot/certbot:latest
    volumes:
      - ./certs:/etc/letsencrypt
      - ./certbot:/var/www/certbot
    command: >
      certonly --webroot -w /var/www/certbot
      -d rpal-ide.yourdomain.com
      --email admin@yourdomain.com
      --agree-tos --non-interactive

networks:
  rpal-net:
    driver: bridge
```

**Start:**
```bash
docker-compose up -d
```

---

## 🛡️ Security Configuration

### Environment Variables

Create `.env` in project root:

```env
# Server
RPAL_ENV=production
RPAL_IDE_HOST=0.0.0.0
RPAL_IDE_PORT=8787

# SSL/TLS
RPAL_USE_SSL=true
RPAL_SSL_CERT=/path/to/cert.pem
RPAL_SSL_KEY=/path/to/key.pem

# Security
RPAL_TIMEOUT=30
RPAL_MAX_CODE_SIZE=100
RPAL_ALLOWED_ORIGINS=rpal-ide.yourdomain.com
```

### Security Headers (Automatic)

The server automatically adds:
- ✅ `Strict-Transport-Security` - Force HTTPS
- ✅ `X-Content-Type-Options: nosniff` - Prevent MIME sniffing
- ✅ `X-Frame-Options: DENY` - Prevent clickjacking
- ✅ `X-XSS-Protection` - Enable XSS filtering
- ✅ `Content-Security-Policy` - Control resource loading

---

## 🐳 Quick Deploy (DigitalOcean, AWS, etc.)

### 1. Create Ubuntu VM
```bash
ssh root@your-server-ip
```

### 2. Install Docker
```bash
curl -fsSL https://get.docker.com -o get-docker.sh
sh get-docker.sh
```

### 3. Clone and deploy
```bash
git clone https://github.com/yourusername/RPAL_Compiler.git
cd RPAL_Compiler
docker-compose up -d
```

### 4. Setup DNS
Point your domain to server IP

### 5. Get certificate
```bash
docker-compose run certbot
```

---

## ✅ Verification Checklist

- [ ] Certificate is valid: `curl -I https://your-domain/`
- [ ] HSTS header present: `curl -I https://your-domain/ | grep HSTS`
- [ ] HTTP redirects to HTTPS: `curl -I http://your-domain/`
- [ ] API responds: `curl -X POST https://your-domain/api/run`
- [ ] Security headers: `curl -I https://your-domain/ | grep X-`

---

## 🔧 Troubleshooting

**Certificate not found?**
```bash
ls -la web/server/.certs/
python web/server/generate_cert.py
```

**Port already in use?**
```bash
lsof -i :8787  # Find process
kill -9 <PID>  # Kill it
```

**Nginx 502 Bad Gateway?**
```bash
sudo systemctl status nginx
sudo nginx -t  # Check config
```

**SSL verification fails?**
```bash
openssl x509 -in cert.pem -text -noout
```

---

## 📚 References

- [Let's Encrypt](https://letsencrypt.org/)
- [Nginx SSL Configuration](https://nginx.org/en/docs/http/ngx_http_ssl_module.html)
- [Mozilla SSL Configuration Generator](https://ssl-config.mozilla.org/)
- [OWASP Security Headers](https://owasp.org/www-project-secure-headers/)

---

## 🎯 Next: CI/CD Pipeline

Once HTTPS is working, set up automated deployment:
- See `CICD_SETUP.md` for GitHub Actions
- Automatic SSL renewal
- Zero-downtime deployments
