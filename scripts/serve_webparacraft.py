"""
Local dev server for the webparacraft repo (default ~/webparacraft).

index.html picks its resource root from the hostname, so the repo is served in both layouts:
  - localhost / 127.0.0.1: everything is loaded from /npl/webparacraft/ and /ServiceWorker.js
    is registered with scope "/"; that worker forwards every other path to the in-page
    NPL WebServer.
  - any other host (LAN IP): everything is loaded from / and the worker is registered as
    the extensionless ./ServiceWorker.
    /ServiceWorker.js, /ServiceWorker  -> <root>/ServiceWorker.js
    /npl/webparacraft/<path>           -> <root>/<path>
    /<path>                            -> <root>/<path>
COOP/COEP headers are added so SharedArrayBuffer works for the multi-thread build.

A LAN address over plain http is not a secure context: the browser disables the service
worker, SharedArrayBuffer and navigator.xr there. Pass --cert/--key (e.g. made by mkcert)
to serve https. See docs/web-dev-workflow.md.

usage: python3 scripts/serve_webparacraft.py [--root ~/webparacraft] [--port 8088]
                                             [--host 0.0.0.0] [--cert c.pem --key k.pem]
"""
import argparse
import http.server
import os
import socket
import socketserver
import ssl
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
        if url_path in ("/ServiceWorker", "/ServiceWorker.js"):
            return super().translate_path("/ServiceWorker.js")
        if url_path.startswith(PREFIX):
            # the base class strips the query and drops ".." segments
            return super().translate_path("/" + url_path[len(PREFIX):])
        return super().translate_path(path)

    def end_headers(self):
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cache-Control", "no-cache, no-store, must-revalidate")
        super().end_headers()


def lan_ip():
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.connect(("10.255.255.255", 1))  # no packet is sent
            return s.getsockname()[0]
    except OSError:
        return None


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--root", default=os.path.expanduser("~/webparacraft"))
    parser.add_argument("--port", type=int, default=8088)
    parser.add_argument("--host", default="127.0.0.1", help="0.0.0.0 to allow LAN access")
    parser.add_argument("--cert", help="TLS certificate (PEM) to serve https")
    parser.add_argument("--key", help="TLS private key (PEM)")
    args = parser.parse_args()

    root = os.path.abspath(os.path.expanduser(args.root))
    if not os.path.isfile(os.path.join(root, "index.html")):
        raise SystemExit(f"{root} does not look like a webparacraft checkout (index.html missing)")

    def handler(*a, **k):
        return Handler(*a, directory=root, **k)

    socketserver.ThreadingTCPServer.allow_reuse_address = True
    with socketserver.ThreadingTCPServer((args.host, args.port), handler) as httpd:
        scheme = "http"
        if args.cert:
            context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
            context.load_cert_chain(args.cert, args.key)
            httpd.socket = context.wrap_socket(httpd.socket, server_side=True)
            scheme = "https"
        print(f"Serving {root}")
        print(f"  this machine: {scheme}://127.0.0.1:{args.port}{PREFIX}index.html")
        if args.host != "127.0.0.1":
            ip = lan_ip()
            if ip:
                print(f"  LAN:          {scheme}://{ip}:{args.port}/index.html")
            if scheme == "http":
                print("  note: plain http on a LAN address has no service worker, SharedArrayBuffer or WebXR")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":
    main()
