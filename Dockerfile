# Stage 1: compile the RPAL interpreter from source
FROM gcc:13 AS builder

WORKDIR /src
COPY main.cpp ./
COPY lexer/ ./lexer/
COPY parser/ ./parser/
COPY standardizer/ ./standardizer/
COPY flattener/ ./flattener/
COPY cse_machine/ ./cse_machine/
COPY Makefile ./

RUN make

# Stage 2: Python runtime
FROM python:3.13-slim

WORKDIR /app

# Install security updates
RUN apt-get update && apt-get upgrade -y && rm -rf /var/lib/apt/lists/*

# Copy freshly compiled binary from builder
COPY --from=builder /src/rpal20 /app/compiler/bin/rpal20

# Copy web server
COPY web/server/ /app/server/
COPY web/public/ /app/public/

WORKDIR /app/server

# Install dependencies and create non-root user
RUN pip install --no-cache-dir -r requirements.txt \
    && useradd -m -u 1000 rpal && chown -R rpal:rpal /app
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

