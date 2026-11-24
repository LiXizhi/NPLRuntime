import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "scripts"))
from serve_webparacraft import build_disk_manifest


class LocalAssetCacheTest(unittest.TestCase):
    def test_only_current_cached_assets_are_mounted(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp).resolve()
            bucket = root / "temp/cache/a"
            bucket.mkdir(parents=True)
            digest = "ab" * 16
            name = digest + "12"
            # Desktop .z entries are already decompressed in the native cache.
            (bucket / name).write_bytes(b"decoded model")
            (bucket / (digest + "10")).write_bytes(b"old version")
            (bucket / "notes").write_bytes(b"unrelated")
            manifest = root / "assets_manifest.txt"
            manifest.write_text(f"model/a.x.z,{digest},12\nmodel/b.x.z,{digest},12\n")
            entries = build_disk_manifest(str(root), ["script"], "")
            cached = [(path, size) for path, size, _ in entries if path.startswith("temp/")]
            self.assertEqual(cached, [(f"temp/cache/a/{name}", len(b"decoded model"))])
            manifest.unlink()
            self.assertFalse(any(p.startswith("temp/") for p, *_ in
                                 build_disk_manifest(str(root), ["script"], "")))


if __name__ == "__main__":
    unittest.main()
