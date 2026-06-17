# RPAL IDE - Cloud Deployment Guide

Comprehensive step-by-step guides for deploying RPAL IDE to major cloud platforms.

---

## 🎯 Quick Comparison

| Platform | Cost | Setup Time | Best For | SSL |
|----------|------|-----------|----------|-----|
| **Heroku** | $5-50/mo | 5 min | Quick prototyping | ✅ Free |
| **Azure** | $10-100/mo | 10 min | Enterprise | ✅ Managed |
| **AWS EC2** | $5-50/mo | 15 min | Scalability | ✅ Self/ACM |
| **Docker Hub** | Free | 5 min | Simplicity | ❌ Needs proxy |
| **DigitalOcean App** | $5-40/mo | 10 min | Developers | ✅ Free |

---

## 1️⃣ Heroku Deployment (Easiest)

**Time: 5 minutes | Cost: Free → $5+/mo | Best for: Quick demos**

### Prerequisites
- Heroku account (free)
- Git installed
- Heroku CLI: `https://devcenter.heroku.com/articles/heroku-cli`

### Steps

1. **Login to Heroku**
```bash
heroku login
```

2. **Create Heroku app**
```bash
heroku create rpal-ide
```

3. **Add Procfile** (already created)
```bash
echo "web: cd web/server && python app.py" > Procfile
```

4. **Configure environment**
```bash
heroku config:set RPAL_IDE_HOST=0.0.0.0
heroku config:set RPAL_IDE_PORT=8787
heroku config:set RPAL_ENV=production
```

5. **Deploy**
```bash
git add .
git commit -m "Ready for Heroku"
git push heroku main
```

6. **View logs**
```bash
heroku logs --tail
```

7. **Access**
```
https://rpal-ide.herokuapp.com/
```

### SSL
✅ **Automatic** - Heroku provides free SSL for *.herokuapp.com

### Scaling
```bash
heroku ps:scale web=2  # Multiple dynos
```

---

## 2️⃣ Azure Container Instances (Best Balance)

**Time: 10 minutes | Cost: $5-30/mo | Best for: Enterprise**

### Prerequisites
- Azure account (free trial: $200 credit)
- Azure CLI: `https://learn.microsoft.com/cli/azure/install-azure-cli`
- Docker Hub account (free)

### Steps

1. **Build and push Docker image**
```bash
docker build -t yourusername/rpal-ide:latest .
docker push yourusername/rpal-ide:latest
```

2. **Login to Azure**
```bash
az login
```

3. **Create resource group**
```bash
az group create --name rpal-rg --location eastus
```

4. **Deploy container**
```bash
az container create \
  --resource-group rpal-rg \
  --name rpal-ide \
  --image yourusername/rpal-ide:latest \
  --cpu 1 --memory 1 \
  --environment-variables \
    RPAL_IDE_HOST=0.0.0.0 \
    RPAL_IDE_PORT=8787 \
  --ports 80 443 \
  --dns-name-label rpal-ide \
  --restart-policy OnFailure
```

5. **Get URL**
```bash
az container show \
  --resource-group rpal-rg \
  --name rpal-ide \
  --query ipAddress.fqdn
```

### SSL
✅ **Azure Application Gateway** (recommended)
- Add custom domain
- Auto-renews with Let's Encrypt

Or use **Nginx reverse proxy** with container

---

## 3️⃣ AWS EC2 (Most Control)

**Time: 15 minutes | Cost: $5-50/mo | Best for: Scalability**

### Prerequisites
- AWS account (free tier: 12 months)
- AWS CLI configured
- Key pair created

### Steps

1. **Launch EC2 instance**
```bash
aws ec2 run-instances \
  --image-ids ami-0c55b159cbfafe1f0 \
  --instance-type t2.micro \
  --key-name your-key-pair \
  --security-groups allow-80-443 \
  --tag-specifications 'ResourceType=instance,Tags=[{Key=Name,Value=rpal-ide}]'
```

2. **Connect via SSH**
```bash
ssh -i your-key.pem ec2-user@your-instance-ip
```

3. **Install Docker**
```bash
sudo yum update -y
sudo yum install -y docker
sudo systemctl start docker
sudo usermod -aG docker $USER
```

4. **Clone and deploy**
```bash
git clone https://github.com/yourusername/RPAL_Compiler.git
cd RPAL_Compiler
docker-compose up -d
```

5. **Setup SSL with Let's Encrypt**
```bash
sudo yum install -y certbot python3-certbot-nginx
sudo certbot certonly --standalone -d rpal.yourdomain.com
```

6. **Configure Nginx** (see web/nginx.conf)
```bash
sudo yum install -y nginx
sudo nano /etc/nginx/conf.d/rpal-ide.conf
# Paste from web/nginx.conf
sudo systemctl start nginx
```

### SSL
✅ **AWS Certificate Manager** (free)
- Issue cert for your domain
- Auto-renews
- Attach to ELB/ALB

Or use **Let's Encrypt** (see above)

---

## 4️⃣ DigitalOcean App Platform (Easiest PaaS)

**Time: 8 minutes | Cost: $5-40/mo | Best for: Developers**

### Prerequisites
- DigitalOcean account ($5 free credit)
- GitHub repo pushed

### Steps

1. **Go to App Platform**
   Dashboard → Apps → Create App

2. **Select source**
   Choose your GitHub repo

3. **Configure**
   - Source: Branch (main)
   - Resource: Basic ($5/mo)
   - Port: 8787
   - Environment:
     ```
     RPAL_ENV=production
     RPAL_IDE_HOST=0.0.0.0
     ```

4. **Deploy**
   Click "Deploy App"

5. **Add custom domain**
   Apps → Your App → Settings → Domains

### SSL
✅ **Automatic** - DigitalOcean provides free SSL

---

## 5️⃣ Docker Hub + Watchtower (CI/CD)

**Time: 10 minutes | Cost: Free | Best for: Auto-updates**

### Prerequisites
- Docker Hub account (free)
- Watchtower installed on server

### Steps

1. **Build and push**
```bash
docker build -t yourusername/rpal-ide:latest .
docker push yourusername/rpal-ide:latest
```

2. **Run with Watchtower** (auto-updates)
```bash
docker run -d \
  -e RPAL_IDE_HOST=0.0.0.0 \
  -p 8787:8787 \
  yourusername/rpal-ide:latest

# Auto-update container when image changes
docker run -d \
  -v /var/run/docker.sock:/var/run/docker.sock \
  containrrr/watchtower \
  yourusername/rpal-ide \
  --cleanup
```

3. **Setup reverse proxy** (local Nginx)
```bash
docker run -d \
  -p 80:80 -p 443:443 \
  -v ./web/nginx.conf:/etc/nginx/conf.d/default.conf:ro \
  -v ./certs:/etc/nginx/certs:ro \
  nginx:alpine
```

---

## 🔐 SSL/TLS for Each Platform

### Heroku
- Automatic (*.herokuapp.com)
- Custom domain: Add DNS CNAME record
- Use custom domain cert

### Azure
- Azure Application Gateway
- Azure Key Vault
- Let's Encrypt via certbot

### AWS
- AWS Certificate Manager (free)
- Attach to CloudFront/ALB/NLB
- Auto-renewal

### DigitalOcean
- Automatic managed cert
- Custom domain support
- Auto-renewal

---

## 📊 Monitoring & Logging

### Azure
```bash
az container logs --resource-group rpal-rg --name rpal-ide --tail 20
```

### AWS
```bash
aws logs tail /aws/ec2/rpal-ide --follow
```

### Heroku
```bash
heroku logs --tail
```

### DigitalOcean
Dashboard → Apps → Logs tab

---

## 🚀 Performance Tips

1. **Use CDN** for static assets
2. **Enable gzip compression** (in nginx.conf ✓)
3. **Add caching headers**
4. **Monitor memory usage**
5. **Scale horizontally** when needed

---

## 💰 Cost Optimization

| Platform | Base | Scale |
|----------|------|-------|
| Heroku | $5/mo | +$5 per dyno |
| Azure | $10/mo | +$10 per instance |
| AWS | $5/mo | +$0.01/GB |
| DigitalOcean | $5/mo | +$5 per app |

---

## 🛠️ Troubleshooting

**Can't connect to /api/run?**
- Check firewall rules
- Verify port in security group
- Check docker logs

**High memory usage?**
- Increase container memory limit
- Check for resource leaks
- Monitor interpreter processes

**SSL certificate error?**
- Verify domain DNS
- Check cert expiration
- Renew certificates

---

## 📚 Next Steps

1. Choose your platform (Heroku = easiest)
2. Follow step-by-step guide above
3. Test at your deployed URL
4. Setup CI/CD pipeline (see CICD_SETUP.md)
5. Monitor performance (see MONITORING.md)

---

## 🎓 Additional Resources

- [Heroku Docs](https://devcenter.heroku.com/)
- [Azure Container Docs](https://docs.microsoft.com/azure/container-instances/)
- [AWS EC2 Tutorial](https://docs.aws.amazon.com/AWSEC2/)
- [DigitalOcean Apps](https://docs.digitalocean.com/products/app-platform/)
- [Docker Hub](https://hub.docker.com/)
