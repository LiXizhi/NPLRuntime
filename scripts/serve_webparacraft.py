"""
Local dev server for the webparacraft repo (default ~/webparacraft).

On localhost, index.html loads everything from /npl/webparacraft/ and registers
/ServiceWorker.js with scope "/", so the repo must be served with this layout:
    /ServiceWorker.js          -> <root>/ServiceWorker.js
    /npl/webparacraft/<path>   -> <root>/<path>
Every other path returns 404. COOP/COEP headers are added so SharedArrayBuffer
works for the multi-thread build. See docs/web-dev-workflow.md.

usage: python3 scripts/serve_webparacraft.py [--root ~/webparacraft] [--port 8088]
"""
import argparse
import http.server
import os
import socketserver
import urllib.parse

PREFIX = "/npl/webparacraft/"


class Handler(http.server.SimpleHTTPRequestHandler):
    extensions_map = {
        **http.server.SimpleHTTPRequestHandler.extensions_map,
        ".wasm": "application/wasm",
        ".js": "text/javascript",
        ".data": "application/octet-stream",
    }

    def translate_path(self, path):
        url_path = urllib.parse.urlsplit(path).path
        if url_path == "/ServiceWorker.js":
            return super().translate_path(url_path)
        if url_path.startswith(PREFIX):
            # the base class strips the query and drops ".." segments
            return super().translate_path("/" + url_path[len(PREFIX):])
        return os.path.join(self.directory, ".not_found")

    def end_headers(self):
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cache-Control", "no-cache, no-store, must-revalidate")
        super().end_headers()


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--root", default=os.path.expanduser("~/webparacraft"))
    parser.add_argument("--port", type=int, default=8088)
    parser.add_argument("--host", default="127.0.0.1")
    args = parser.parse_args()

    root = os.path.abspath(os.path.expanduser(args.root))
    if not os.path.isfile(os.path.join(root, "index.html")):
        raise SystemExit(f"{root} does not look like a webparacraft checkout (index.html missing)")

    def handler(*a, **k):
        return Handler(*a, directory=root, **k)

    socketserver.ThreadingTCPServer.allow_reuse_address = True
    with socketserver.ThreadingTCPServer((args.host, args.port), handler) as httpd:
        print(f"Serving {root}")
        print(f"  open http://127.0.0.1:{args.port}{PREFIX}index.html", flush=True)
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":
    main()
