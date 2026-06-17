FROM python:3.11-slim

WORKDIR /app

# Install security updates
RUN apt-get update && apt-get upgrade -y && rm -rf /var/lib/apt/lists/*

# Copy compiler binary
COPY compiler/bin/rpal20* /app/compiler/bin/

# Copy web server
COPY web/server/ /app/server/
COPY web/public/ /app/public/

WORKDIR /app/server

# Install dependencies
RUN pip install --no-cache-dir -r requirements.txt

# Create non-root user for security
RUN useradd -m -u 1000 rpal && chown -R rpal:rpal /app
USER rpal

# Expose port (8787)
EXPOSE 8787

# Default configuration
ENV RPAL_ENV=production
ENV RPAL_IDE_PORT=8787
ENV RPAL_IDE_HOST=0.0.0.0
ENV RPAL_USE_SSL=false

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD python -c "import urllib.request; urllib.request.urlopen('http://localhost:8787/').read()" || exit 1

# Run server
CMD ["python", "app.py"]

