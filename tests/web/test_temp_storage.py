"""Run against a local Web build: python3 tests/web/test_temp_storage.py (Playwright + Chrome)."""
import argparse
import hashlib
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import threading
import time
from urllib.parse import urlsplit

from playwright.sync_api import sync_playwright

ROOT = Path(__file__).resolve().parents[2]
ASSET = b"downloaded manifest payload"
DIGEST = hashlib.md5(ASSET).hexdigest()
CACHE_PATH = f"/temp/cache/{DIGEST[0]}/{DIGEST}{len(ASSET)}"
HTML = b"""<!doctype html><canvas id="canvas"></canvas><script>
var Module = {
  canvas: document.querySelector('canvas'), noInitialRun: true,
  preRun: [() => {
    FS.mkdirTree('/temp/preloaded');
    FS.writeFile('/temp/preloaded/seed', 'bundle');
    FS.writeFile('/temp/preloaded/deleted', 'bundle');
    FS.mkdirTree('/temp/haqi-wardrobe/packages');
    FS.mount(IDBFS, {autoPersist: true}, '/temp/haqi-wardrobe/packages');
  }],
  onRuntimeInitialized() {
    window.sync = populate => new Promise((ok,fail) => FS.syncfs(populate, e => e ? fail(e) : ok()));
    window.loaded = sync(true);
  }
};
</script><script src="/ENGINE.js"></script>"""


def regression(engine, threaded=False):
    target = "ParaCraft" if threaded else "ParaCraftSingleThread"
    requests = []

    class Handler(BaseHTTPRequestHandler):
        def do_GET(self):
            path = urlsplit(self.path).path
            if path == "/":
                data, mime, cache = HTML.replace(b"/ENGINE.js", f"/{target}.js".encode()), "text/html", "no-store"
            elif path == "/art":
                requests.append(path)
                data, mime, cache = ASSET, "application/octet-stream", "public, max-age=86400"
            elif path in (f"/{target}.js", f"/{target}.wasm", f"/{target}.worker.js"):
                data = (engine / path[1:]).read_bytes()
                mime = "application/wasm" if path.endswith(".wasm") else "application/javascript"
                cache = "public, max-age=86400"
            else:
                self.send_error(404)
                return
            self.send_response(200)
            self.send_header("Content-Type", mime)
            self.send_header("Cross-Origin-Opener-Policy", "same-origin")
            self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
            self.send_header("Content-Length", str(len(data)))
            self.send_header("Cache-Control", cache)
            self.end_headers()
            self.wfile.write(data)

        def log_message(self, *args):
            pass

    server = ThreadingHTTPServer(("127.0.0.1", 0), Handler)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    origin = f"http://127.0.0.1:{server.server_port}"

    def open_game(page):
        page.goto(origin)
        page.wait_for_function("window.loaded !== undefined")
        page.evaluate("window.loaded")

    def fetch_art(page):
        return page.evaluate("""async path => {
          FS.mkdirTree(path.slice(0, path.lastIndexOf('/')));
          FS.lookupPath('/temp').node.mount.type.excludeAsset(path);
          FS.writeFile(path, new Uint8Array(await (await fetch('/art')).arrayBuffer()));
          return FS.readFile(path, {encoding:'utf8'});
        }""", CACHE_PATH)

    def keys(page):
        return page.evaluate("""() => new Promise((resolve, reject) => {
          const r = indexedDB.open('/temp');
          r.onerror = () => reject(r.error);
          r.onsuccess = () => {
            const db = r.result, keys = db.transaction('FILE_DATA').objectStore('FILE_DATA').getAllKeys();
            keys.onsuccess = () => { db.close(); resolve(keys.result); };
          };
        })""")

    try:
        with sync_playwright() as pw:
            browser = pw.chromium.launch(channel="chrome")
            ctx = browser.new_context()
            page = ctx.new_page()
            open_game(page)
            assert fetch_art(page) == ASSET.decode()
            page.evaluate("""p => {
              FS.mkdirTree('/temp/work/sub');
              FS.writeFile('/temp/work/sub/data','draft');
              FS.rename('/temp/work/sub','/temp/moved');
              FS.writeFile('/temp/preloaded/seed','edited');
              FS.unlink('/temp/preloaded/deleted');
              FS.writeFile('/temp/cache/notes','plain');
              FS.writeFile(p + '1','ordinary hash-shaped file');
              FS.writeFile(p + '.tmp','ordinary');
              FS.symlink('/temp/moved/data','/temp/link');
              FS.symlink('/temp/missing','/temp/dangling');
              FS.writeFile('/temp/haqi-wardrobe/packages/installed','wardrobe');
            }""", CACHE_PATH)
            deadline = time.monotonic() + 5
            while "/temp/moved/data" not in keys(page):
                if time.monotonic() >= deadline:
                    raise AssertionError("ordinary writes were not persisted automatically")
                page.wait_for_timeout(25)
            saved = keys(page)
            assert CACHE_PATH not in saved
            assert CACHE_PATH + ".tmp" in saved and CACHE_PATH + "1" in saved
            assert "/temp/moved/data" in saved and "/temp/work/sub/data" not in saved
            assert "/temp/haqi-wardrobe/packages/installed" not in saved

            # A second tab must not overwrite unrelated keys from its older snapshot.
            second = ctx.new_page()
            open_game(second)
            page.evaluate("FS.writeFile('/temp/tab-one', '1')")
            second.evaluate("FS.writeFile('/temp/tab-two', '2')")
            page.evaluate("sync(false)")
            second.evaluate("sync(false)")
            second.close()
            open_game(page)
            assert page.evaluate("FS.readlink('/temp/link')") == "/temp/moved/data"
            assert page.evaluate("FS.readlink('/temp/dangling')") == "/temp/missing"
            restored = page.evaluate("""p => {
              const read = name => FS.readFile(name, {encoding:'utf8'});
              return [read('/temp/link'), read('/temp/preloaded/seed'), read('/temp/cache/notes'),
                read(p + '.tmp'), read('/temp/haqi-wardrobe/packages/installed'),
                read('/temp/tab-one'), read('/temp/tab-two'), FS.analyzePath(p).exists,
                FS.analyzePath('/temp/preloaded/deleted').exists];
            }""", CACHE_PATH)
            assert restored == ["draft", "edited", "plain", "ordinary", "wardrobe", "1", "2", False, False], restored
            assert fetch_art(page) == ASSET.decode() and requests == ["/art"], requests
            browser.close()
    finally:
        server.shutdown()
        server.server_close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--engine-dir", type=Path, default=ROOT / "web/auto_generate_files")
    parser.add_argument("--threaded", action="store_true", help="exercise the ParaCraft pthread build")
    args = parser.parse_args()
    regression(args.engine_dir, args.threaded)
    print("PASS: Web /temp persistence, packaged defaults, nested mount, concurrent tabs and HTTP cache reuse")
