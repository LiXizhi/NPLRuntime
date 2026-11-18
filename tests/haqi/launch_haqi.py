"""Run Haqi with this checkout's built WASM engine and an external webparacraft shell."""

import argparse
import hashlib
import http.server
import json
from pathlib import Path
import re
import socket
import sys
import urllib.error
import urllib.parse
import urllib.request
import webbrowser


REPOSITORY = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPOSITORY / "scripts"))
import serve_webparacraft as web_server


MANIFEST = "/auto_generate_files/paracraft_sitefile_md5sums.js"
STATUS = "/__haqi_test__/config"
ENGINE_FILES = ("ParaCraftSingleThread.js", "ParaCraftSingleThread.wasm")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", choices=("webgl", "webgpu"), default="webgl")
    parser.add_argument("--web-root", type=Path, default=REPOSITORY.parent / "webparacraft")
    parser.add_argument("--engine-dir", type=Path, default=REPOSITORY / "web/auto_generate_files")
    parser.add_argument("--port", type=int, help="Loopback port (default 8091, or 8092 for the regression)")
    parser.add_argument("--test-missing-region", action="store_true",
                        help="Load the terrain-region regression world instead of logging in")
    parser.add_argument("--no-browser", action="store_true", help="Serve and print the URL without opening it")
    args = parser.parse_args()
    root = args.web_root.resolve()
    engine = args.engine_dir.resolve()
    for name in ENGINE_FILES:
        if not (engine / name).is_file():
            parser.error(f"Missing {engine / name}. Build WASM with scripts/dev_web.ps1 build first.")
    for name in ("haqi1.html", MANIFEST.lstrip("/"),
                 "auto_generate_files/haqi1_paracraft_asset.js",
                 "auto_generate_files/haqi1_paracraft_asset.data",
                 "auto_generate_files/paracraft_asset_script.js",
                 "auto_generate_files/paracraft_asset_script.data",
                 "auto_generate_files/paracraft_asset_temp.js",
                 "auto_generate_files/paracraft_asset_temp.data"):
        if not (root / name).is_file():
            parser.error(f"Missing {root / name}. Set --web-root to a webparacraft checkout with Haqi packages.")

    configuration = {"version": 1, "web_root": str(root), "engine_dir": str(engine)}
    port = args.port if args.port is not None else (8092 if args.test_missing_region else 8091)
    base_url = f"http://127.0.0.1:{port}"
    renderer = "opengl" if args.renderer == "webgl" else "webgpu"
    query = {"renderer": renderer}
    if args.test_missing_region:
        fixture = Path(__file__).resolve().parent / "fixtures/missing_region"
        query.update(searchroot=fixture.as_posix(), searchpath="script/apps/Aries/main_loop.lua,terrain", mcp=base_url)
    url = f"{base_url}{web_server.PREFIX}haqi1.html?" + urllib.parse.urlencode(query, quote_via=urllib.parse.quote)

    def engine_manifest():
        text = (root / MANIFEST.lstrip("/")).read_text(encoding="utf-8")
        for name in ENGINE_FILES:
            digest = hashlib.md5((engine / name).read_bytes()).hexdigest()
            pattern = r'(\["auto_generate_files/' + re.escape(name) + r'"\]:\s*")[^"]+'
            text, count = re.subn(pattern, lambda match: match.group(1) + digest, text)
            if count != 1:
                raise ValueError(f"Expected one hash manifest entry for {name}")
        return text.encode("utf-8")

    try:
        engine_manifest()
    except (OSError, ValueError) as error:
        parser.error(str(error))

    class Handler(web_server.Handler):
        server_config = web_server.ServerConfig(str(root), str(engine), "", "", False)

        def __init__(self, *handler_args, **handler_kwargs):
            super().__init__(*handler_args, directory=str(root), **handler_kwargs)

        def relative_route(self, path):
            route = urllib.parse.urlsplit(path).path
            if route.startswith(web_server.PREFIX):
                route = "/" + route[len(web_server.PREFIX):]
            return route

        def translate_path(self, path):
            route = self.relative_route(path)
            if route in ("/auto_generate_files/" + name for name in ENGINE_FILES):
                return str(engine / Path(route).name)
            return super().translate_path(path)

        def _route_special_endpoints(self):
            route = self.relative_route(self.path)
            if route == STATUS:
                self._send_json(200, configuration)
                return True
            if route == MANIFEST:
                body = engine_manifest()
                self.send_response(200)
                self.send_header("Content-Type", "text/javascript; charset=utf-8")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                if self.command != "HEAD":
                    self.wfile.write(body)
                return True
            return super()._route_special_endpoints()

    class TestServer(http.server.ThreadingHTTPServer):
        def server_bind(self):
            # Windows SO_REUSEADDR permits two live listeners on the same port.
            if hasattr(socket, "SO_EXCLUSIVEADDRUSE"):
                self.allow_reuse_address = False
                self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_EXCLUSIVEADDRUSE, 1)
            super().server_bind()

    try:
        httpd = TestServer(("127.0.0.1", port), Handler)
    except OSError as error:
        # Reuse only a test server serving the same shell and engine directories.
        try:
            with urllib.request.urlopen(base_url + STATUS, timeout=2) as response:
                running_configuration = json.load(response)
        except (OSError, ValueError, urllib.error.URLError):
            running_configuration = None
        if running_configuration != configuration:
            parser.error(f"Cannot start on port {port}: {error}. Use --port with another free port.")
        print(f"Reusing Haqi test server.\n{url}", flush=True)
        if not args.no_browser:
            webbrowser.open(url)
        return

    with httpd:
        print(f"Haqi shell: {root}\nLocal engine: {engine}\n{url}\nPress Ctrl+C to stop.", flush=True)
        if not args.no_browser:
            webbrowser.open(url)
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":
    main()
