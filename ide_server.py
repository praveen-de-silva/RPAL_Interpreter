from __future__ import annotations

import json
import os
import subprocess
import tempfile
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path


ROOT = Path(__file__).resolve().parent


def find_interpreter() -> Path | None:
    if os.name == "nt":
        candidates = [
            ROOT / "rpal20.exe",
            ROOT / "rpal20",
            ROOT.parent / "rpal20.exe",
            ROOT.parent / "rpal20",
        ]
    else:
        candidates = [
            ROOT / "rpal20",
            ROOT.parent / "rpal20",
            ROOT / "rpal20.exe",
            ROOT.parent / "rpal20.exe",
        ]
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    return None


class Handler(BaseHTTPRequestHandler):
    def log_message(self, format: str, *args) -> None:  # noqa: A003
        return

    def _send_json(self, status: int, payload: dict) -> None:
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _send_html(self, status: int, html: str) -> None:
        body = html.encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self) -> None:  # noqa: N802
        if self.path in {"/", "/rpal_ide.html"}:
            self._send_html(200, (ROOT / "rpal_ide.html").read_text(encoding="utf-8"))
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
        try:
            payload = json.loads(self.rfile.read(content_length).decode("utf-8"))
        except json.JSONDecodeError:
            self._send_json(400, {"error": "Invalid JSON body"})
            return

        code = str(payload.get("code", ""))
        mode = str(payload.get("mode", "output"))
        flag = {"ast": "-ast", "st": "-st"}.get(mode)

        with tempfile.NamedTemporaryFile("w", suffix=".rpal", delete=False, encoding="utf-8", dir=ROOT) as temp_file:
            temp_file.write(code)
            temp_path = Path(temp_file.name)

        try:
            command = [str(interpreter)]
            if flag:
                command.append(flag)
            command.append(str(temp_path))

            completed = subprocess.run(command, capture_output=True, cwd=str(ROOT), timeout=30)
            stdout = completed.stdout.decode("utf-8", errors="replace") if completed.stdout else ""
            stderr = completed.stderr.decode("utf-8", errors="replace") if completed.stderr else ""
            if completed.returncode != 0:
                self._send_json(500, {"error": stderr.strip() or stdout.strip() or f"Interpreter exited with code {completed.returncode}"})
                return

            self._send_json(200, {"output": stdout, "binary": interpreter.name})
        except subprocess.TimeoutExpired:
            self._send_json(504, {"error": "Interpreter timed out after 30 seconds"})
        except OSError as exc:
            self._send_json(500, {"error": f"Could not execute interpreter: {exc}"})
        finally:
            try:
                os.remove(temp_path)
            except OSError:
                pass


def main() -> None:
    port = int(os.environ.get("RPAL_IDE_PORT", "8787"))
    server = ThreadingHTTPServer(("127.0.0.1", port), Handler)
    print(f"RPAL IDE running at http://127.0.0.1:{port}/")
    server.serve_forever()


if __name__ == "__main__":
    main()
