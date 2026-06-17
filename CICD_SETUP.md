# CI/CD Pipeline - GitHub Actions

Automated build, test, and deployment pipeline for RPAL IDE using GitHub Actions.

## 📋 What This Does

The pipeline automatically:

1. **Build & Test** (on every push)
   - Install dependencies
   - Run linting (flake8)
   - Run security scans (bandit)
   - Build Docker image

2. **Push Image** (on merge to main)
   - Push Docker image to GitHub Container Registry
   - Auto-tag with commit SHA and 'latest'

3. **Deploy to Heroku** (on merge to main)
   - Auto-deploy to Heroku
   - Run smoke tests
   - Notify on failure

4. **Notifications** (on failure)
   - Optional Slack notifications

## 🚀 Setup

### Step 1: Repository Structure (✅ Already Done)

```
.github/workflows/
└── deploy.yml        ← GitHub Actions workflow
```

### Step 2: Add Secrets to GitHub

Go to: **Settings → Secrets and variables → Actions**

Add these secrets:

#### For Heroku Deployment
```
HEROKU_API_KEY          (your Heroku API token)
HEROKU_APP_NAME         (your app name, e.g., rpal-ide)
```

**Get Heroku API Key:**
```bash
# Terminal
heroku auth:token

# Or: https://dashboard.heroku.com/account/applications/authorizations
```

#### Optional: For Slack Notifications
```
SLACK_WEBHOOK           (Slack webhook URL)
```

**Get Slack Webhook:**
1. Go to: https://api.slack.com/apps
2. Create New App → From scratch
3. Incoming Webhooks → Add New Webhook to Workspace
4. Copy Webhook URL

### Step 3: Create Deployment Credentials

#### GitHub Container Registry Token
```bash
# Generate personal access token
# GitHub Settings → Developer settings → Personal access tokens → Tokens (classic)
# Scopes: write:packages, read:packages
```

This is automatic - uses GITHUB_TOKEN.

### Step 4: Push to GitHub

```bash
git add .github/workflows/deploy.yml
git commit -m "Add CI/CD pipeline"
git push origin main
```

## 📊 Pipeline Status

After push, check status:

**GitHub Dashboard:**
- Go to **Actions** tab
- See real-time build status
- Click on workflow for details

**Command Line:**
```bash
# View workflow status
gh run list

# View specific run
gh run view <run-id>
```

## 🔄 Workflow Files

### `.github/workflows/deploy.yml`

**Triggers:**
- On every push to `main` or `develop`
- On pull requests to `main`

**Jobs:**

#### 1. build-and-test
```yaml
- Runs on: ubuntu-latest
- Python: 3.11
- Steps:
  - Checkout code
  - Install dependencies
  - Lint (flake8)
  - Security scan (bandit)
  - Build Docker image
```

#### 2. push-image
```yaml
- Runs on: ubuntu-latest (if main branch)
- Pushes to: GitHub Container Registry
- Tags: latest, <commit-sha>
```

#### 3. deploy-heroku
```yaml
- Runs on: ubuntu-latest (if main branch)
- Deploys to: Heroku
- Tests: Smoke test (curl endpoint)
```

#### 4. notify
```yaml
- Runs on: All (always)
- Sends: Slack notification on failure
```

## 📝 Customize Workflow

### Deploy to Different Platform

**Azure:**
```yaml
- name: Deploy to Azure
  uses: azure/webapps-deploy@v2
  with:
    app-name: ${{ secrets.AZURE_APP_NAME }}
    publish-profile: ${{ secrets.AZURE_PUBLISH_PROFILE }}
```

**AWS:**
```yaml
- name: Deploy to AWS Lambda
  uses: aws-actions/lambda-deploy@v1
  with:
    function-name: rpal-ide
```

**DigitalOcean:**
```yaml
- name: Deploy to DigitalOcean
  uses: appleboy/ssh-action@master
  with:
    host: ${{ secrets.DO_HOST }}
    username: ${{ secrets.DO_USER }}
    key: ${{ secrets.DO_SSH_KEY }}
    script: |
      cd /app
      docker pull ${{ env.IMAGE }}
      docker-compose up -d
```

### Skip Deployment

Add `[skip deploy]` to commit message:
```bash
git commit -m "Update docs [skip deploy]"
```

### Manual Trigger

Add this section to workflow:
```yaml
on:
  push:
    branches: [main]
  workflow_dispatch:  # Manual trigger
```

Then use GitHub CLI:
```bash
gh workflow run deploy.yml
```

## 🔐 Security Best Practices

1. **Secrets Management**
   - Use GitHub Secrets (not in code)
   - Rotate API keys regularly
   - Use branch protection rules

2. **Code Review**
   - Require PR reviews before merge
   - Run tests on PRs (automatic)

3. **Artifact Security**
   - Sign container images
   - Scan for vulnerabilities

4. **Access Control**
   - Limit who can approve deployments
   - Use environment protection rules

**Environment Protection:**
```yaml
deploy:
  environment:
    name: production
    url: https://rpal-ide.herokuapp.com
```

Then in Settings → Environments → Add protection rules

## 📊 Monitoring

### GitHub Actions Logs

```bash
# List recent runs
gh run list

# View logs for specific run
gh run view <id> --log

# Follow in real-time
gh run view <id> --log --follow
```

### Heroku Deployment

```bash
# After deploy, check logs
heroku logs --tail

# Check app status
heroku ps
```

### Container Registry

```bash
# List images
docker images | grep rpal-ide

# Pull image
docker pull ghcr.io/yourusername/RPAL_Compiler/rpal-ide:latest
```

## 🐛 Troubleshooting

**Build fails with "ImportError"?**
- Check `web/server/requirements.txt`
- Ensure all dependencies listed
- Run locally: `pip install -r web/server/requirements.txt`

**Heroku deployment fails?**
- Verify HEROKU_API_KEY in secrets
- Check HEROKU_APP_NAME exists
- Run: `heroku auth:whoami` locally

**Docker push fails?**
- GITHUB_TOKEN needs `write:packages` scope
- Check Container Registry permissions
- Verify image name in workflow

**Smoke test fails?**
- App may take time to start
- Increase sleep time in workflow
- Check app logs: `heroku logs --tail`

## ✅ Verification

After first deployment:

1. **Check workflow ran:**
   ```bash
   gh run list
   ```

2. **Verify deployment:**
   ```bash
   curl https://rpal-ide.herokuapp.com/
   ```

3. **Check image in registry:**
   ```bash
   docker pull ghcr.io/yourusername/RPAL_Compiler/rpal-ide:latest
   ```

4. **View app logs:**
   ```bash
   heroku logs --tail
   ```

## 📚 Example Workflow

```
1. Developer pushes to main
   ↓
2. GitHub Actions triggered
   ↓
3. Build & test runs (2-3 min)
   ↓
4. If passes, image pushed to registry
   ↓
5. Deploy to Heroku (1-2 min)
   ↓
6. Smoke test runs
   ↓
7. App live! ✅
```

**Total time: 5-8 minutes**

## 🎯 Next Steps

1. ✅ Workflow configured
2. Add secrets (HEROKU_API_KEY, etc.)
3. Push to GitHub
4. Watch Actions tab
5. Verify deployment

## 📖 References

- [GitHub Actions Docs](https://docs.github.com/actions)
- [Workflow Syntax](https://docs.github.com/actions/using-workflows/workflow-syntax-for-github-actions)
- [Using Secrets](https://docs.github.com/actions/security-guides/encrypted-secrets)
- [Docker Build Action](https://github.com/docker/build-push-action)

## 💡 Pro Tips

1. **Use branch protection rules** to prevent merging without tests
2. **Add deployment statuses** to PRs for visibility
3. **Use concurrency** to cancel old builds when new push happens
4. **Cache dependencies** to speed up builds
5. **Sign container images** for production security

Example caching:
```yaml
- uses: actions/cache@v3
  with:
    path: |
      ~/.cache/pip
      ~/.cache/docker
    key: ${{ runner.os }}-${{ hashFiles('**/requirements.txt') }}
```

---

**Pipeline ready! Every push to main now auto-deploys to production.** 🚀
