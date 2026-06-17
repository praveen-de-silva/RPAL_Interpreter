# Heroku Deployment Guide

Quick 5-minute deployment to Heroku for RPAL IDE.

## Prerequisites

1. **Heroku Account** - Sign up at https://heroku.com (free)
2. **Heroku CLI** - Download from https://devcenter.heroku.com/articles/heroku-cli
3. **Git** - Already have this in your repo
4. **Docker Hub account** (optional, for pre-built images)

## Step-by-Step

### 1. Install Heroku CLI

**Windows:**
```powershell
# Download installer from https://devcenter.heroku.com/articles/heroku-cli
# Or use scoop
scoop install heroku
```

**macOS:**
```bash
brew install heroku/brew/heroku
```

**Linux:**
```bash
curl https://cli-assets.heroku.com/install.sh | sh
```

### 2. Login to Heroku

```bash
heroku login
# Opens browser - login with your credentials
```

Verify:
```bash
heroku auth:whoami
```

### 3. Create Heroku App

```bash
# From your rpal20 directory
cd /path/to/rpal20

# Create new app
heroku create rpal-ide

# Or use your own name
heroku create my-rpal-compiler
```

### 4. View App Info

```bash
heroku apps:info rpal-ide
```

Output:
```
=== rpal-ide
Git URL:       https://git.heroku.com/rpal-ide.git
Web URL:       https://rpal-ide.herokuapp.com/
Owner Email:   your@email.com
```

### 5. Configure Environment

```bash
# Set environment variables
heroku config:set RPAL_ENV=production
heroku config:set RPAL_IDE_HOST=0.0.0.0
heroku config:set RPAL_IDE_PORT=8787
heroku config:set RPAL_TIMEOUT=30

# Verify
heroku config
```

### 6. Deploy

```bash
# Add all files
git add .

# Commit
git commit -m "Deploy to Heroku"

# Push to Heroku
git push heroku main

# Watch build
heroku logs --tail
```

You'll see:
```
Launching... done, v5
https://rpal-ide.herokuapp.com/ deployed to Heroku
```

### 7. Access Your App

```bash
# Open in browser
heroku open

# Or manually visit
https://rpal-ide.herokuapp.com/
```

### 8. View Logs

```bash
# Last 50 lines
heroku logs

# Follow in real-time
heroku logs --tail

# Filter by source
heroku logs --source app
heroku logs --source heroku
```

## 🔐 SSL/TLS

✅ **Automatic** - All *.herokuapp.com domains have free SSL

To use custom domain:
```bash
# Add custom domain
heroku domains:add rpal-ide.yourdomain.com

# Configure DNS (point to Heroku)
# Add CNAME: rpal-ide.yourdomain.com -> rpal-ide.herokuapp.com

# Verify
heroku domains

# SSL auto-issued
```

## 🚀 Scaling

```bash
# Check current dynos
heroku ps

# Scale to 2 web dynos ($7/mo each)
heroku ps:scale web=2

# Scale down
heroku ps:scale web=1
```

## 📊 Monitoring

```bash
# View metrics
heroku metrics

# Check dyno usage
heroku metrics:dyno

# Memory and CPU
heroku ps:exec --dyno web.1
```

## 🐛 Troubleshooting

**App crashes on startup?**
```bash
heroku logs --tail --source app
# Look for Python errors
```

**ImportError: No module named 'config'?**
```bash
# config.py is optional - falls back to defaults
# Verify it's in web/server/
ls web/server/config.py
```

**Port binding issues?**
```bash
# Heroku assigns PORT via environment
# Our app reads RPAL_IDE_PORT but should use $PORT
# This is handled in config.py - check it
```

**High memory usage?**
```bash
# Upgrade dyno type
heroku ps:resize web=standard-1x

# Or reduce processes
```

## 💰 Pricing

| Dyno Type | Cost | RAM | CPU |
|-----------|------|-----|-----|
| Free | $0 | 512MB | Shared |
| Eco | $5/mo | 512MB | Shared |
| Basic | $7/mo | 512MB | 1x |
| Standard | $50/mo | 2GB | 4x |

**Recommended for production:** Standard or Eco

## 🔄 Continuous Deployment

### Option 1: Auto-deploy from GitHub

```bash
# Enable auto-deploy
heroku apps:join rpal-ide
# Via dashboard: Settings → Deployment method → GitHub
```

### Option 2: GitHub Actions (see CICD_SETUP.md)

```yaml
- name: Deploy to Heroku
  run: |
    git push https://heroku:${{ secrets.HEROKU_API_KEY }}@git.heroku.com/rpal-ide.git main
```

## 📝 Common Commands

```bash
# Restart app
heroku restart

# Run one-off command
heroku run "python --version"

# SSH into dyno
heroku ps:exec

# Open app
heroku open

# View config
heroku config

# Remove app
heroku apps:destroy rpal-ide
```

## ✅ Verification Checklist

- [ ] App deployed: `heroku open`
- [ ] SSL working: `curl -I https://rpal-ide.herokuapp.com`
- [ ] API responds: `curl -X POST https://rpal-ide.herokuapp.com/api/run`
- [ ] Logs accessible: `heroku logs`
- [ ] Metrics available: `heroku metrics`

## 🎉 Done!

Your RPAL IDE is now live on:
```
https://rpal-ide.herokuapp.com/
```

Share it with others!

## 📚 Next

- [Heroku Docs](https://devcenter.heroku.com/)
- [Custom Domain Setup](https://devcenter.heroku.com/articles/custom-domains)
- [CI/CD Pipeline](../CICD_SETUP.md)
