# ✅ Heroku Deployment - Step by Step

You created a Heroku account ✓ Here's what to do next!

## Step 1: Install Heroku CLI

### Windows
```powershell
# Option 1: Using scoop (recommended if you have it)
scoop install heroku

# Option 2: Download installer
# Visit: https://devcenter.heroku.com/articles/heroku-cli
# Download and run the .exe file

# Verify installation
heroku --version
```

### macOS
```bash
brew install heroku
heroku --version
```

### Linux
```bash
curl https://cli-assets.heroku.com/install.sh | sh
heroku --version
```

---

## Step 2: Login to Heroku

```bash
heroku login
```

**What happens:**
- Browser opens
- Click "Log In"
- Enter your email & password
- You'll see "Logged in as yourname@email.com"

**Verify:**
```bash
heroku auth:whoami
# Should show your email
```

---

## Step 3: Prepare Git Repository

```bash
# Navigate to project
cd C:\Users\ASUS\Desktop\PL_Project\rpal20

# Initialize git (if not already done)
git init

# Add all files
git add .

# Commit
git commit -m "Initial RPAL IDE setup - ready for Heroku"

# Check status
git status
```

---

## Step 4: Create Heroku App

```bash
# Create app
heroku create rpal-ide

# You'll see:
# Creating ⬢ rpal-ide... done
# https://rpal-ide.herokuapp.com/ | https://git.heroku.com/rpal-ide.git
```

**Note:** If "rpal-ide" is taken, use your own name:
```bash
heroku create my-rpal-compiler
```

---

## Step 5: Deploy to Heroku

```bash
# This deploys your code to Heroku
git push heroku main

# Watch the build happen in your terminal
# Takes 2-3 minutes first time
# You'll see:
# Building...
# Slug compiled to ...
# Launching...
# Deployed to Heroku!
```

---

## Step 6: View Your App

### Option A: Open in browser
```bash
heroku open
```

### Option B: Manual URL
```
https://rpal-ide.herokuapp.com/
```

### Option C: Get app info
```bash
heroku apps:info rpal-ide
```

---

## Step 7: Check Logs

```bash
# View latest logs
heroku logs

# Follow logs in real-time (watch mode)
heroku logs --tail

# Ctrl+C to stop watching
```

---

## ✅ Verification Checklist

- [ ] Heroku CLI installed (`heroku --version`)
- [ ] Logged in (`heroku auth:whoami`)
- [ ] App created (`heroku create rpal-ide`)
- [ ] Code pushed (`git push heroku main`)
- [ ] App loads (`heroku open` or manual URL)
- [ ] API works (try `/api/run` endpoint)

---

## 🐛 Troubleshooting

### "Heroku CLI not found"
- Restart terminal/PowerShell after installation
- Verify path includes Heroku: `where heroku`

### "Authentication failed"
```bash
heroku logout
heroku login
```

### "App won't start"
```bash
# Check logs for errors
heroku logs --tail

# Common fixes:
# 1. Check Procfile exists
# 2. Check requirements.txt
# 3. Port must be 8787 (already set)
```

### "Git push fails"
```bash
# Check remotes
git remote -v

# Should show heroku remote
# If not, add it:
git remote add heroku https://git.heroku.com/rpal-ide.git

# Then retry
git push heroku main
```

### "Port binding error"
- Already handled in code (respects PORT env var)
- Check that RPAL_IDE_PORT is not hardcoded

---

## 🎉 Success!

Your app is now live at:
```
https://rpal-ide.herokuapp.com/
```

### Next: Setup Auto-Deploy (CI/CD)

So you don't have to manually push every time:

1. Push code to GitHub
2. Add GitHub secrets:
   - `HEROKU_API_KEY` (from `heroku auth:token`)
   - `HEROKU_APP_NAME` (rpal-ide)
3. GitHub Actions auto-deploys!

See: `CICD_SETUP.md`

---

## 💡 Useful Commands

```bash
# View app info
heroku apps:info rpal-ide

# View environment variables
heroku config

# Set environment variable
heroku config:set RPAL_ENV=production

# Run command on Heroku
heroku run "python --version"

# SSH into app
heroku ps:exec

# View processes
heroku ps

# Scale to 2 dynos
heroku ps:scale web=2

# Open app in browser
heroku open

# View logs
heroku logs --tail

# Restart app
heroku restart

# Destroy app (if you want to delete)
heroku apps:destroy rpal-ide
```

---

## 📊 Your Heroku App is Free For:

- First 550 hours/month (free tier)
- 1 web dyno (sleeps after 30 min inactivity)
- Upgrade anytime for $7-50/month for always-on

---

## 🎯 You Did It!

RPAL IDE is now hosted on the internet! 🚀

Share: `https://rpal-ide.herokuapp.com/`
