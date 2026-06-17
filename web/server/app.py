from __future__ import annotations

import json
import os
import subprocess
import tempfile
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

try:
    from config import (
        HOST, PORT, USE_SSL, SECURITY_HEADERS, ALLOWED_ORIGINS,
        TIMEOUT_SECONDS, MAX_CODE_SIZE_KB, get_ssl_context
    )
except ImportError:
    HOST = os.getenv("RPAL_IDE_HOST", "127.0.0.1")
    PORT = int(os.getenv("RPAL_IDE_PORT", "8787"))
    USE_SSL = False
    SECURITY_HEADERS = {}
    ALLOWED_ORIGINS = ["localhost"]
    TIMEOUT_SECONDS = 30
    MAX_CODE_SIZE_KB = 100
    get_ssl_context = lambda: None


ROOT = Path(__file__).resolve().parent.parent
COMPILER_BIN = ROOT.parent / "compiler" / "bin"
PUBLIC_DIR = ROOT / "public"


def find_interpreter() -> Path | None:
    candidates = [
        COMPILER_BIN / "rpal20.exe",
        COMPILER_BIN / "rpal20",
        ROOT.parent.parent / "rpal20.exe",
        ROOT.parent.parent / "rpal20",
    ]
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    return None


class Handler(BaseHTTPRequestHandler):
    def log_message(self, format: str, *args) -> None:  # noqa: A003
        return

    def _add_security_headers(self) -> None:
        """Add security headers to response"""
        for header, value in SECURITY_HEADERS.items():
            if value:
                self.send_header(header, value)

    def _send_json(self, status: int, payload: dict) -> None:
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self._add_security_headers()
        self.end_headers()
        self.wfile.write(body)

    def _send_html(self, status: int, html: str) -> None:
        body = html.encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self._add_security_headers()
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self) -> None:  # noqa: N802
        if self.path in {"/", "/index.html"}:
            index_file = PUBLIC_DIR / "index.html"
            if index_file.is_file():
                self._send_html(200, index_file.read_text(encoding="utf-8"))
                return
            self._send_json(404, {"error": "index.html not found"})
            return
        self._send_json(404, {"error": "Not found"})

    def do_POST(self) -> None:  # noqa: N802
        if self.path != "/api/run":
            self._send_json(404, {"error": "Not found"})
            return

        interpreter = find_interpreter()
        if interpreter is None:
            self._send_json(500, {"error": "Compiled interpreter not found. Build the project first."})
            return

        content_length = int(self.headers.get("Content-Length", "0"))
        if content_length > MAX_CODE_SIZE_KB * 1024:
            self._send_json(413, {"error": f"Code too large (max {MAX_CODE_SIZE_KB}KB)"})
            return

        try:
            payload = json.loads(self.rfile.read(content_length).decode("utf-8"))
        except json.JSONDecodeError:
            self._send_json(400, {"error": "Invalid JSON body"})
            return

        code = str(payload.get("code", ""))
        mode = str(payload.get("mode", "output"))
        flag = {"ast": "-ast", "st": "-st"}.get(mode)

        with tempfile.NamedTemporaryFile("w", suffix=".rpal", delete=False, encoding="utf-8", dir=COMPILER_BIN) as temp_file:
            temp_file.write(code)
            temp_path = Path(temp_file.name)

        try:
            command = [str(interpreter)]
            if flag:
                command.append(flag)
            command.append(str(temp_path))

            completed = subprocess.run(command, capture_output=True, cwd=str(COMPILER_BIN), timeout=TIMEOUT_SECONDS)
            stdout = completed.stdout.decode("utf-8", errors="replace") if completed.stdout else ""
            stderr = completed.stderr.decode("utf-8", errors="replace") if completed.stderr else ""
            if completed.returncode != 0:
                self._send_json(500, {"error": stderr.strip() or stdout.strip() or f"Interpreter exited with code {completed.returncode}"})
                return

            self._send_json(200, {"output": stdout, "binary": interpreter.name})
        except subprocess.TimeoutExpired:
            self._send_json(504, {"error": f"Interpreter timed out after {TIMEOUT_SECONDS} seconds"})
        finally:
            try:
                os.remove(temp_path)
            except OSError:
                pass


def main() -> None:
    ssl_context = get_ssl_context()
    protocol = "https" if USE_SSL else "http"
    
    server = ThreadingHTTPServer((HOST, PORT), Handler)
    if ssl_context:
        server.socket = ssl_context.wrap_socket(server.socket, server_side=True)
    
    print(f"🚀 RPAL IDE running at {protocol}://{HOST}:{PORT}/")
    if USE_SSL:
        print("🔐 SSL/TLS enabled")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n✋ Shutting down...")
        server.shutdown()


if __name__ == "__main__":
    main()