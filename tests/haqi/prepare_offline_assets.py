"""Prepare the public artwork for the disconnected Haqi town fixture."""

import argparse
import json
import re
from concurrent.futures import ThreadPoolExecutor
from hashlib import md5, sha256
from io import BytesIO
from pathlib import Path, PurePosixPath
from tempfile import TemporaryDirectory
from urllib.parse import quote
from urllib.request import urlopen
from zipfile import ZIP_DEFLATED, ZipFile

from launch_haqi_native import package_files

HERE = Path(__file__).resolve().parent
WORLD = "worlds/myworlds/61haqitown/"
ASSETS = "https://cdn.keepwork.com/update61/assetdownload/update/"
REFERENCE = re.compile(
    rb"[a-zA-Z0-9_.\\/\-]+\.(?:dds|png|jpg|x|xml|ogg|wav)\b", re.IGNORECASE
)


def catalog_for(web):
    manifest = package_files(web, "paracraft_asset_script")["assets_manifest.txt"]
    catalog = {}
    for line in manifest.decode("utf-8-sig").splitlines():
        fields = line.rsplit(",", 2)
        if len(fields) != 3:
            continue
        name, digest, size = fields
        path = PurePosixPath(name)
        if (
            name.endswith((".p", ".z"))
            and re.fullmatch("[0-9a-f]{32}", digest)
            and size.isdecimal()
            and not path.is_absolute()
            and ".." not in path.parts
            and ":" not in name
            and "\\" not in name
        ):
            catalog[name[:-2].lower()] = (name, digest, int(size))
    return catalog, sha256(manifest).hexdigest()


def prepare(web, paraworld, archive, native_cache=None, download=True):
    catalog, manifest_hash = catalog_for(web)
    recipe = (HERE / "offline/assets.txt").read_text().splitlines()
    needed = {key for key in recipe if key and not key.startswith("#")}
    unknown = needed - catalog.keys()
    if unknown:
        raise ValueError(f"Unlisted required asset in this edition: {min(unknown)}")
    needed.update(k for k in catalog if k.startswith(WORLD))
    for config in (
        "WorldData/61HaqiTown.NPC.xml",
        "Spells/Fire_SingleAttack_Level1.xml",
        "Spells/Action_QuestAcquire.xml",
    ):
        data = (paraworld / "config/Aries" / config).read_bytes()
        needed.update(
            v.decode().replace("\\", "/").lower()
            for v in REFERENCE.findall(data)
            if v.decode().replace("\\", "/").lower() in catalog
        )
    archive.parent.mkdir(parents=True, exist_ok=True)
    cache = archive.parent / "public-cache"
    cache.mkdir(exist_ok=True)

    def load(key):
        name, digest, size = catalog[key]
        destination = cache / (digest + str(size))
        if native_cache:
            cached = native_cache / digest[0] / destination.name
            if cached.is_file():
                return cached.read_bytes()
        if destination.is_file():
            packed = destination.read_bytes()
        else:
            if not download:
                raise ValueError(
                    f"Missing cached public asset {key}. Prepare once with network access."
                )
            with urlopen(
                ASSETS + quote(f"{name},{digest},{size}", safe="/,"), timeout=45
            ) as response:
                packed = response.read()
        if len(packed) != size or md5(packed).hexdigest() != digest:
            raise ValueError(f"Invalid public asset {key}")
        if not destination.exists():
            with TemporaryDirectory(dir=cache) as d:
                f = Path(d) / "download"
                f.write_bytes(packed)
                f.replace(destination)
        if name.endswith(".z"):
            with ZipFile(BytesIO(packed)) as source:
                return source.read(source.namelist()[0])
        return packed

    with (
        TemporaryDirectory(dir=archive.parent) as d,
        ThreadPoolExecutor(max_workers=8) as pool,
    ):
        package = Path(d) / "town.zip"
        with ZipFile(package, "w", compression=ZIP_DEFLATED, compresslevel=1) as output:
            seen = set()
            while needed:
                batch = sorted(needed)
                seen.update(batch)
                needed = set()
                for key, data in zip(batch, pool.map(load, batch)):
                    output.writestr(key, data)
                    if key.endswith((".x", ".xml", ".lua", ".txt")):
                        parent = key.rpartition("/")[0]
                        for raw in REFERENCE.findall(data):
                            ref = raw.decode().replace("\\", "/").lower()
                            for candidate in (ref, parent + "/" + ref):
                                if candidate in catalog and candidate not in seen:
                                    needed.add(candidate)
            output.writestr(
                "haqi-test/artwork.json",
                json.dumps({"manifest": manifest_hash, "files": len(seen)}),
            )
        package.replace(archive)
    return len(seen)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--web-root", required=True, type=Path)
    parser.add_argument("--paraworld-root", required=True, type=Path)
    parser.add_argument("--archive", required=True, type=Path)
    parser.add_argument(
        "--native-cache", type=Path, help="Optional native runtime temp/cache"
    )
    parser.add_argument("--no-download", action="store_true")
    args = parser.parse_args()
    print(
        prepare(
            args.web_root,
            args.paraworld_root,
            args.archive,
            args.native_cache,
            not args.no_download,
        ),
        "assets prepared",
        flush=True,
    )


if __name__ == "__main__":
    main()
