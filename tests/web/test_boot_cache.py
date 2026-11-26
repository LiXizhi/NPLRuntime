"""Run with a built Web engine, Python Playwright and Chrome: python3 tests/web/test_boot_cache.py."""
import shutil
import sys
import tempfile
import threading
from http.server import ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit

from playwright.sync_api import sync_playwright

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))
from serve_webparacraft import Handler, ServerConfig

WASM = "ParaCraftSingleThread.wasm"


def mark_engine(wasm, marker):
    # Two same-sized builds, distinguished by a custom section on the compiled module.
    if marker == b"A":
        with wasm.open("ab") as stream:
            stream.write(b"\x00\x07\x05probe" + marker)
    else:
        with wasm.open("r+b") as stream:
            stream.seek(-1, 2)
            stream.write(marker)


def observe_engine(page):
    page.add_init_script("""for (const name of ['instantiateStreaming', 'instantiate']) {
        const instantiate = WebAssembly[name];
        WebAssembly[name] = async function (...args) {
            const result = await instantiate.apply(this, args);
            const module = result.module || (args[0] instanceof WebAssembly.Module && args[0]);
            if (module) {
                const [probe] = WebAssembly.Module.customSections(module, 'probe');
                if (probe) window.engineProbe = new TextDecoder().decode(probe);
            }
            return result;
        };
    }""")


def regression(tmp):
    web = ROOT / "web"
    assets = tmp / "assets"
    (assets / "script").mkdir(parents=True)
    script = assets / "script/probe.lua"
    script.write_text("first")
    wasm = tmp / WASM
    shutil.copyfile(web / "auto_generate_files" / WASM, wasm)
    mark_engine(wasm, b"A")
    requests = []

    class Server(Handler):
        server_config = ServerConfig(str(web), "", str(assets), "", False)

        def __init__(self, *args, **kwargs):
            super().__init__(*args, directory=str(web), **kwargs)

        def translate_path(self, path):
            if urlsplit(path).path.endswith("/" + WASM):
                return str(wasm)
            return super().translate_path(path)

        def log_request(self, code="-", size="-"):
            requests.append((urlsplit(self.path).path, code))

        def log_message(self, *args):
            pass

        def _handle_fs_log(self):
            self.rfile.read(int(self.headers.get("Content-Length", 0)))
            self._send_json(200, {"ok": True})

    with ThreadingHTTPServer(("127.0.0.1", 0), Server) as server, sync_playwright() as pw:
        threading.Thread(target=server.serve_forever, daemon=True).start()
        origin = f"http://127.0.0.1:{server.server_port}"
        url = origin + "/npl/webparacraft/index.html?searchpath=script/"
        try:
            for run in ("cold", "restart", "edit", "updated restart", "low quota"):
                if run == "edit":
                    mark_engine(wasm, b"B")
                    script.write_text("newer")
                profile = tmp / ("small-profile" if run == "low quota" else "profile")
                context = pw.chromium.launch_persistent_context(str(profile), channel="chrome", headless=True)
                try:
                    page = context.pages[0]
                    observe_engine(page)
                    if run == "low quota":
                        cdp = context.new_cdp_session(page)
                        cdp.send("Storage.overrideQuotaForOrigin", {"origin": origin, "quotaSize": 1024 * 1024})
                    requests.clear()
                    page.goto(url, wait_until="domcontentloaded")
                    page.wait_for_function("window.Module && Module.isRuntimeInitialized", timeout=90000)
                    engine = page.evaluate("window.engineProbe")
                    assert engine == ("A" if run in ("cold", "restart") else "B"), (run, engine)
                    actual = page.evaluate("FS.readFile('/script/probe.lua', {encoding:'utf8'})")
                    assert actual == ("first" if run in ("cold", "restart") else "newer"), (run, actual)
                    status = [code for path, code in requests if path.endswith("/" + WASM)]
                    batches = sum(path == "/fs/batch" for path, _ in requests)
                    warm = run in ("restart", "updated restart")
                    assert status == ([304] if warm else [200]), (run, status)
                    assert (batches == 0) == warm, (run, batches)
                    if run == "low quota":
                        quota = cdp.send("Storage.getUsageAndQuota", {"origin": origin})
                        assert quota["overrideActive"] and quota["quota"] == 1024 * 1024, quota
                        assert not page.evaluate("""async () => {
                            const cache = await caches.open('WebParacraft-local-large-v1');
                            return (await cache.keys()).some(k => k.url.endsWith('.wasm'));
                        }""")
                    else:
                        page.wait_for_function("""async size => {
                            const cache = await caches.open('WebParacraft-local-large-v1');
                            const keys = (await cache.keys()).filter(k => k.url.endsWith('.wasm'));
                            if (keys.length !== 1) return false;
                            return Number((await cache.match(keys[0])).headers.get('Content-Length')) === size;
                        }""", arg=wasm.stat().st_size)
                    print(f"{run}: engine {engine}, WASM HTTP {status[0]}, boot batch requests {batches}, script {actual}")
                finally:
                    context.close()
        finally:
            server.shutdown()


if __name__ == "__main__":
    with tempfile.TemporaryDirectory() as tmp:
        regression(Path(tmp))
