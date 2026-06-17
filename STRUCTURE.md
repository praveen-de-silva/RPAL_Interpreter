# RPAL Project - Structured Directory

This is the reorganized RPAL Compiler project with a modern, production-ready structure.

## 📦 Directory Layout

```
rpal20/
├── compiler/                   # Compiler & interpreter
│   ├── bin/                    # Compiled binaries (rpal20.exe, rpal20)
│   ├── src/                    # C++ source code (main.cpp, etc.)
│   └── README.md              # Compiler documentation
│
├── web/                        # Web IDE & server
│   ├── public/                 # Frontend assets
│   │   ├── index.html         # Main IDE interface
│   │   └── assets/
│   │       ├── css/           # Custom stylesheets
│   │       ├── js/            # Custom JavaScript
│   │       └── images/        # Images & icons
│   ├── server/                 # Backend server
│   │   ├── app.py             # HTTP server (Python)
│   │   └── requirements.txt    # Python dependencies
│   ├── .env.example           # Environment template
│   └── README.md              # Web app documentation
│
├── lexer/                      # Lexical analyzer
├── parser/                     # Parser
├── standardizer/               # Standardizer
├── flattener/                  # Flattener
├── cse_machine/                # CSE Machine implementation
├── Tests/                      # Test suite
├── docs/                       # Documentation
│
├── Dockerfile                  # Container configuration
├── docker-compose.yml          # Multi-container setup
├── Makefile                    # Build automation
├── README.md                   # Project README
└── .gitignore                  # Git ignore rules
```

## 🚀 Quick Start

### Run Web IDE (Recommended for Hosting)

```bash
# Local development
cd web/server
pip install -r requirements.txt
python app.py
# Open http://localhost:8787/

# Or use Docker
docker-compose up -d
# Open http://localhost:8787/
```

### Run Compiler Tests

```bash
python run_tests.ps1  # Windows PowerShell
make test            # Unix/Linux
```

## 📚 Documentation

- **Web IDE**: See `web/README.md`
- **Compiler**: See `compiler/README.md`
- **Full Docs**: See `docs/` folder

## 🔑 Key Files

- `web/public/index.html` - IDE interface
- `web/server/app.py` - Backend server
- `compiler/bin/rpal20.exe` - Windows interpreter
- `compiler/bin/rpal20` - Unix interpreter

## 🐳 Deployment

### Docker (Recommended)
```bash
docker build -t rpal-ide .
docker run -p 8787:8787 rpal-ide
```

### Manual
```bash
cd web/server
pip install -r requirements.txt
RPAL_IDE_PORT=8787 RPAL_IDE_HOST=0.0.0.0 python app.py
```

## 📝 Configuration

Create `.env` in project root:
```env
RPAL_IDE_PORT=8787
RPAL_IDE_HOST=0.0.0.0
```

## 🎯 Next Steps

1. ✅ Structure organized
2. ⏭️ Build/test compiler (`make build`)
3. ⏭️ Deploy web IDE (see deployment options above)
4. ⏭️ Configure domain & SSL (for production)

## 👥 Team

Built by Bulagala & Praveen
