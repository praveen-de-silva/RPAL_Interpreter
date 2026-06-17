# 🚀 RPAL IDE - Deployment Checklist & Quick Reference

Complete guide to get RPAL IDE live in production.

---

## ✅ Pre-Deployment Checklist

- [ ] Project structured properly (see `STRUCTURE.md`)
- [ ] SSL certificates ready (`config.py`, `generate_cert.py`)
- [ ] Docker builds successfully (`docker build -t rpal-ide .`)
- [ ] Environment variables configured (`.env.example`)
- [ ] GitHub Actions workflow created (`.github/workflows/deploy.yml`)
- [ ] Documentation reviewed (guides in `/docs` and root)

---

## 🎯 Choose Your Path

### Path A: Local Development (5 minutes)
**Best for:** Testing before deployment

```bash
# 1. Build and run locally
docker-compose up -d

# 2. Access
http://localhost:8787/

# 3. View logs
docker-compose logs -f

# 4. Stop
docker-compose down
```

**Files used:**
- `docker-compose.yml`
- `Dockerfile`
- `web/public/index.html`
- `web/server/app.py`

---

### Path B: Heroku Deployment (10 minutes) ⭐ EASIEST
**Best for:** Quick production deployment

```bash
# 1. Login
heroku login

# 2. Create app
heroku create rpal-ide

# 3. Deploy
git add .
git commit -m "Deploy to Heroku"
git push heroku main

# 4. View logs
heroku logs --tail

# 5. Access
https://rpal-ide.herokuapp.com/
```

**Files used:**
- `Procfile`
- `runtime.txt`
- `Dockerfile`
- `web/server/requirements.txt`

**SSL:** ✅ Automatic (*.herokuapp.com)

**Reference:** `docs/HEROKU_DEPLOY.md`

---

### Path C: Azure Deployment (10 minutes)
**Best for:** Enterprise deployments

```bash
# 1. Build Docker image
docker build -t rpal-ide:latest .

# 2. Push to registry
# (Docker Hub / Azure Container Registry)

# 3. Create Azure resources
az group create --name rpal-rg --location eastus

# 4. Deploy container
az container create \
  --resource-group rpal-rg \
  --name rpal-ide \
  --image yourreg.azurecr.io/rpal-ide:latest

# 5. Get URL
az container show --resource-group rpal-rg --name rpal-ide
```

**SSL:** ✅ Azure Application Gateway

**Reference:** `CLOUD_DEPLOY.md` (Option 2)

---

### Path D: AWS EC2 (15 minutes)
**Best for:** Maximum control and scalability

```bash
# 1. Launch EC2 instance
# Dashboard: EC2 > Launch instances > t2.micro

# 2. SSH into instance
ssh -i key.pem ec2-user@instance-ip

# 3. Install Docker
sudo yum install -y docker
sudo systemctl start docker

# 4. Clone and run
git clone <your-repo>
cd RPAL_Compiler
docker-compose up -d

# 5. Setup Nginx + Let's Encrypt
# (See CLOUD_DEPLOY.md)
```

**SSL:** ✅ Let's Encrypt or AWS Certificate Manager

**Reference:** `CLOUD_DEPLOY.md` (Option 3)

---

### Path E: DigitalOcean (8 minutes)
**Best for:** Developer-friendly deployment

1. **Create App** → Dashboard → Apps → Create App
2. **Select source** → Connect GitHub repo
3. **Configure**
   - Branch: main
   - Resource: Basic ($5/mo)
   - Port: 8787
4. **Deploy** → Click Deploy
5. **Access** → Your deployed URL

**SSL:** ✅ Automatic

**Reference:** `CLOUD_DEPLOY.md` (Option 4)

---

### Path F: Docker Hub (5 minutes) + Watchtower
**Best for:** Simple auto-updating deployments

```bash
# 1. Build and push
docker build -t yourusername/rpal-ide:latest .
docker push yourusername/rpal-ide:latest

# 2. Deploy anywhere Docker runs
docker run -d \
  -p 8787:8787 \
  -e RPAL_IDE_HOST=0.0.0.0 \
  yourusername/rpal-ide:latest

# 3. Auto-update with Watchtower
docker run -d \
  -v /var/run/docker.sock:/var/run/docker.sock \
  containrrr/watchtower \
  yourusername/rpal-ide \
  --cleanup
```

**SSL:** ❌ Need reverse proxy (Nginx)

**Reference:** `CLOUD_DEPLOY.md` (Option 5)

---

### Path G: Kubernetes (Scalable)
**Best for:** Production at scale

```bash
# 1. Push image to registry
docker push yourreg/rpal-ide:latest

# 2. Create cluster
kubectl create cluster rpal-cluster

# 3. Deploy
kubectl apply -f kubernetes.yaml

# 4. Expose service
kubectl port-forward svc/rpal-ide-service 8787:80

# 5. Access
http://localhost:8787/
```

**SSL:** ✅ Ingress controller (nginx/cert-manager)

**Reference:** `kubernetes.yaml`

---

## 🔐 SSL/HTTPS Setup

### For Development (Self-Signed)

```bash
# Generate certificate
python web/server/generate_cert.py

# Set environment
export RPAL_USE_SSL=true
export RPAL_SSL_CERT=web/server/.certs/rpal_cert.pem
export RPAL_SSL_KEY=web/server/.certs/rpal_key.pem

# Run
python web/server/app.py

# Access (ignore cert warning)
https://127.0.0.1:8787/
```

### For Production (Let's Encrypt)

**Automatic on Heroku & DigitalOcean** ✅

**Manual setup:**
```bash
# Install certbot
sudo apt install certbot certbot-nginx

# Get certificate
sudo certbot certonly --nginx -d yourdomain.com

# Configure nginx (use web/nginx.conf)
# Auto-renewal setup
sudo systemctl enable certbot.timer
```

**Reference:** `HTTPS_SETUP.md`

---

## 🔄 CI/CD Pipeline (Auto-Deploy on Push)

### Setup GitHub Actions

1. **Secrets in GitHub** (Settings → Secrets → Actions)
   ```
   HEROKU_API_KEY          (from: heroku auth:token)
   HEROKU_APP_NAME         (e.g., rpal-ide)
   ```

2. **Workflow file** ✅ Already created
   ```
   .github/workflows/deploy.yml
   ```

3. **Push to trigger**
   ```bash
   git add .
   git commit -m "Trigger CI/CD"
   git push origin main
   ```

4. **Watch deployment**
   - GitHub → Actions tab
   - See real-time build & deploy status
   - Auto-deployed in 5-8 minutes

**Reference:** `CICD_SETUP.md`

---

## 📊 Deployment Comparison

| Aspect | Heroku | Azure | AWS | DO | Docker | K8s |
|--------|--------|-------|-----|-------|--------|-----|
| **Setup Time** | 5m | 10m | 15m | 8m | 5m | 20m |
| **Cost** | $5-50 | $10-100 | $5-50 | $5-40 | $5-20 | $20+ |
| **SSL** | ✅ Auto | ✅ Managed | ✅ ACM | ✅ Auto | ❌ Need proxy | ✅ Cert-mgr |
| **Scaling** | Easy | Medium | Full | Medium | Manual | Auto |
| **Difficulty** | ⭐ Easiest | ⭐⭐ Medium | ⭐⭐⭐ Hard | ⭐⭐ Medium | ⭐ Easy | ⭐⭐⭐ Hard |
| **Auto-Deploy** | ✅ | ✅ | Manual | ✅ | ✅ | ✅ |

**⭐ Recommendation:** Heroku for most users

---

## 🧪 Testing Your Deployment

### Basic Tests

```bash
# 1. Website loads
curl https://your-url/

# 2. SSL valid
curl -I https://your-url/ | grep HSTS

# 3. API works
curl -X POST https://your-url/api/run \
  -H "Content-Type: application/json" \
  -d '{"code":"Print 1","mode":"output"}'

# 4. Security headers
curl -I https://your-url/ | grep X-
```

### Smoke Test Script

```bash
#!/bin/bash
DOMAIN=$1
echo "Testing $DOMAIN..."
curl -f https://$DOMAIN/ || exit 1
curl -f -X POST https://$DOMAIN/api/run \
  -d '{"code":"Print 123","mode":"output"}' \
  -H "Content-Type: application/json" || exit 1
echo "✅ All tests passed!"
```

---

## 🐛 Common Issues & Fixes

| Issue | Solution |
|-------|----------|
| **Port already in use** | `lsof -i :8787` → `kill -9 <PID>` |
| **Cert not found** | Run `python web/server/generate_cert.py` |
| **Heroku app won't start** | `heroku logs --tail` → check errors |
| **High memory usage** | Increase container memory limit |
| **SSL certificate error** | Check domain DNS / cert expiration |
| **API returns 502** | Check backend app logs |
| **Docker build fails** | Ensure `compiler/bin/rpal20*` exists |

---

## 📝 Environment Variables Reference

```env
# Server
RPAL_ENV=production              # development | staging | production
RPAL_IDE_HOST=0.0.0.0           # Listen on all interfaces
RPAL_IDE_PORT=8787              # Port number

# SSL/TLS
RPAL_USE_SSL=false              # Enable HTTPS
RPAL_SSL_CERT=/path/to/cert.pem # Certificate file
RPAL_SSL_KEY=/path/to/key.pem   # Private key file

# Security
RPAL_TIMEOUT=30                 # Execution timeout (seconds)
RPAL_MAX_CODE_SIZE=100          # Max code size (KB)
RPAL_ALLOWED_ORIGINS=*          # CORS origins

# Debugging
RPAL_IDE_DEBUG=false            # Enable debug mode
```

---

## 🚀 Production Best Practices

1. **Use HTTPS** - Always (especially with passwords)
2. **Set environment variables** - Never hardcode secrets
3. **Monitor logs** - Track errors and performance
4. **Setup backups** - In case something breaks
5. **Use CDN** - For static files (CSS, JS)
6. **Enable caching** - Speed up responses
7. **Setup alerts** - Get notified of failures
8. **Rate limiting** - Prevent abuse
9. **Keep updated** - Regular security patches
10. **Load testing** - Verify capacity before launch

---

## 📚 Documentation Index

| Need Help With... | See File |
|------------------|----------|
| SSL/HTTPS setup | `HTTPS_SETUP.md` |
| Cloud deployment | `CLOUD_DEPLOY.md` |
| Heroku specifically | `docs/HEROKU_DEPLOY.md` |
| CI/CD pipeline | `CICD_SETUP.md` |
| Project structure | `STRUCTURE.md` |
| Web app setup | `web/README.md` |

---

## 🎯 Quick Start Command Cheat Sheet

```bash
# Local
docker-compose up -d

# Heroku
heroku login
heroku create rpal-ide
git push heroku main

# Azure
docker build -t rpal-ide .
docker push <registry>/rpal-ide
az container create --image <registry>/rpal-ide --name rpal-ide

# AWS
aws ec2 run-instances --image-id ami-xxx --instance-type t2.micro
ssh -i key.pem ec2-user@instance-ip
docker-compose up -d

# DigitalOcean
# Use dashboard to deploy

# Kubernetes
kubectl apply -f kubernetes.yaml
```

---

## ✅ Final Checklist Before Going Live

- [ ] Project pushed to GitHub
- [ ] GitHub Actions secrets configured
- [ ] Deployment method chosen (Heroku recommended)
- [ ] SSL/HTTPS verified working
- [ ] API endpoint tested (`/api/run`)
- [ ] IDE loads in browser
- [ ] Example programs run successfully
- [ ] Logs accessible
- [ ] Monitoring/alerts setup (if available)
- [ ] Team notified of new URL
- [ ] Custom domain configured (if using)

---

## 🎉 You're Ready!

Pick your deployment path above and follow the steps. Most users:

1. **Test locally** (5 min) - `docker-compose up -d`
2. **Deploy to Heroku** (5 min) - `heroku create && git push heroku main`
3. **Setup CI/CD** (5 min) - Add GitHub secrets, done!

**Total time: 15 minutes from start to live production!**

Questions? See the detailed guides in `/docs` and root directory.

Good luck! 🚀
