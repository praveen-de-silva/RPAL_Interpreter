# RPAL IDE - Web Application

A modern, web-based IDE for writing and executing RPAL (Reduced Programming Language) programs.

## 📁 Project Structure

```
web/
├── public/
│   ├── index.html              # Main IDE interface
│   └── assets/
│       ├── css/                # Stylesheets
│       ├── js/                 # JavaScript files
│       └── images/             # Images and icons
├── server/
│   ├── app.py                  # Flask/HTTP server
│   ├── requirements.txt        # Python dependencies
│   └── config.py               # Configuration (optional)
└── README.md                   # This file
```

## 🚀 Quick Start

### Prerequisites
- Python 3.8+
- Compiled RPAL interpreter (`rpal20.exe` or `rpal20`)
- Docker (optional, for containerized deployment)

### Local Development

1. **Install dependencies**
   ```bash
   cd web/server
   pip install -r requirements.txt
   ```

2. **Run the server**
   ```bash
   python app.py
   ```

3. **Access the IDE**
   - Open browser: `http://localhost:8787/`
   - Port can be changed via `RPAL_IDE_PORT` environment variable

### Environment Variables

Create a `.env` file in the `web/` directory:

```env
RPAL_IDE_PORT=8787
RPAL_IDE_HOST=0.0.0.0
RPAL_IDE_DEBUG=false
```

## 🐳 Docker Deployment

### Build and run with Docker

```bash
# Build image
docker build -t rpal-ide .

# Run container
docker run -p 8787:8787 rpal-ide
```

### Using Docker Compose

```bash
docker-compose up -d
```

The IDE will be available at `http://localhost:8787/`

## 📡 API Endpoints

### GET `/`
Returns the IDE HTML interface.

### POST `/api/run`
Executes RPAL code.

**Request:**
```json
{
  "code": "Print 'Hello World'",
  "mode": "output"  // output | ast | st
}
```

**Response (Success):**
```json
{
  "output": "Hello World",
  "binary": "rpal20.exe"
}
```

**Response (Error):**
```json
{
  "error": "Error message"
}
```

## 🎯 Features

- ✅ Real-time RPAL code execution
- ✅ Three output modes: Output, AST (Abstract Syntax Tree), ST (Standardized Tree)
- ✅ Syntax highlighting ready (extendable)
- ✅ Example programs pre-loaded
- ✅ Responsive design (desktop & mobile)
- ✅ Error reporting
- ✅ Tab support for code indentation

## 🔧 Customization

### Styling
Edit `public/index.html` - CSS variables in `<style>` tag:
- `--bg-0`, `--bg-1`, `--bg-2`: Background colors
- `--accent`, `--accent-2`: Accent colors
- `--text`, `--text-soft`: Text colors

### Adding Examples
In `public/index.html`, add buttons in the examples bar:
```html
<button class="ex-btn" data-code="your code here">Label</button>
```

## 🛠️ Troubleshooting

**"Compiled interpreter not found"**
- Ensure `rpal20.exe` or `rpal20` exists in `../compiler/bin/`

**Port already in use**
- Change `RPAL_IDE_PORT` environment variable
- Or: `netstat -ano | findstr :8787` (Windows)

**CORS issues**
- Server only listens on `127.0.0.1` by default
- Change `RPAL_IDE_HOST` to `0.0.0.0` for external access

## 📝 License

Same as parent project

## 👥 Credits

Built by Bulagala & Praveen
