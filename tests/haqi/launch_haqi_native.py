"""Prepare a native Haqi runtime from local webparacraft packages and launch it."""

import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import subprocess
import sys


REPOSITORY = Path(__file__).resolve().parents[2]


def package_files(root, name):
    """Read file_packager metadata without executing the generated JavaScript."""
    base = root / "auto_generate_files" / name
    text = base.with_suffix(".js").read_text(encoding="utf-8")
    match = re.search(r'loadPackage\((\{"files":.*?\})\);', text, re.S)
    if not match:
        raise ValueError(f"Missing file metadata in {base}.js")
    metadata = json.loads(match.group(1))
    data = base.with_suffix(".data").read_bytes()
    if len(data) != metadata["remote_package_size"]:
        raise ValueError(f"Package size mismatch: {base}.data")
    files = {}
    for entry in metadata["files"]:
        name = entry["filename"].lstrip("/")
        path = PurePosixPath(name)
        start, end = entry["start"], entry["end"]
        if (not name or ".." in path.parts or "\\" in name or ":" in name
                or not 0 <= start <= end <= len(data)):
            raise ValueError(f"Invalid package entry: {name}")
        files[name] = data[start:end]
    return files


def write_file(path, data, preserve=False):
    if path.exists() and (preserve or path.read_bytes() == data):
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)


def prepare_runtime(web_root, paraworld, runtime, manifest_override=None):
    config = paraworld / "config/TaoMee.GameClient.config.xml"
    if not config.is_file() or not (paraworld / "script").is_dir():
        raise ValueError(f"Missing scripts or TaoMee.GameClient.config.xml in {paraworld}")
    haqi = package_files(web_root, "haqi1_paracraft_asset")
    scripts = package_files(web_root, "paracraft_asset_script")
    manifest_source = manifest_override or web_root / "assets_manifest.txt"
    if manifest_override or manifest_source.is_file():
        manifest = manifest_source.read_bytes()
        origin = str(manifest_source)
    else:
        manifest = scripts["assets_manifest.txt"]
        origin = str(web_root / "auto_generate_files/paracraft_asset_script.data")
    if not manifest.strip() or not all(
            re.fullmatch(rb"[^,]+,(?:[0-9a-fA-F]{32},[0-9]+|,)", line.strip())
            for line in manifest.splitlines() if line.strip()):
        raise ValueError(f"Invalid assets_manifest.txt from {origin}")
    for required in ("characters.db", "extendedcost.db.mem", "globalstore.db.mem", "userdata.db"):
        if "config/database/" + required not in haqi:
            raise ValueError(f"Haqi package is missing database {required}")

    runtime.mkdir(parents=True, exist_ok=True)
    # dev points here so Paraworld's older manifest cannot shadow this one.
    # Only the Lua source tree is linked; config and asset packages stay local.
    script_link = runtime / "script"
    source_scripts = paraworld / "script"
    if os.path.lexists(script_link):
        if script_link.resolve() != source_scripts.resolve():
            raise ValueError(f"{script_link} already exists and does not point to {source_scripts}")
    elif os.name == "nt":
        subprocess.run(["cmd", "/c", "mklink", "/J", str(script_link), str(source_scripts)], check=True)
    else:
        script_link.symlink_to(source_scripts, target_is_directory=True)

    for name, data in {**haqi, **scripts}.items():
        # Browser launch metadata and its mc=true config are not native settings.
        if name in ("config.txt", "config/launch_config.txt", "assets_manifest.txt"):
            continue
        write_file(runtime / name, data)
        if name.startswith("config/database/"):
            write_file(runtime / "Database" / Path(name).name, data, preserve=True)
    cache_package = web_root / "auto_generate_files/paracraft_asset_temp.data"
    if cache_package.is_file():
        for name, data in package_files(web_root, "paracraft_asset_temp").items():
            if name.startswith("temp/cache/"):
                write_file(runtime / name, data, preserve=True)
    write_file(runtime / "config/TaoMee.GameClient.config.xml", config.read_bytes())
    write_file(runtime / "assets_manifest.txt", manifest)
    print(f"Game config: {config}")
    print(f"Asset manifest source: {origin}")
    print(f"Asset manifest: {len(manifest.splitlines())} entries; SHA256 {hashlib.sha256(manifest).hexdigest()}")
    print(f"Runtime: {runtime}")


def find_executable():
    if sys.platform == "darwin":
        paths = [REPOSITORY / folder / "Paracraft.app/Contents/MacOS/Paracraft" for folder in
                 ("build/mac/bin", "build-client/bin",
                  "build-client/bin/Release", "build-client/bin/Debug")]
    elif os.name == "nt":
        paths = [REPOSITORY / folder / name for folder in
                 ("build-win64/bin", "build/bin", "ParaWorld/bin64")
                 for name in ("ParaEngineClient.exe", "ParaEngineClient_d.exe")]
    else:
        raise ValueError("This launcher supports macOS and Windows")
    paths = [p for p in paths if p.is_file()]
    if not paths:
        raise ValueError("Native client not found. Build it first or supply --exe.")
    return max(paths, key=lambda p: p.stat().st_mtime)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--web-root", type=Path, default=Path(os.environ.get(
        "WEBPARACRAFT_ROOT", REPOSITORY.parent / "webparacraft")))
    parser.add_argument("--paraworld-root", type=Path, default=Path(os.environ.get(
        "PARAWORLD_ROOT", REPOSITORY.parent / "ParaEngine/paraworld")))
    parser.add_argument("--runtime-dir", type=Path, default=REPOSITORY / "build" /
                        ("haqi-mac-runtime" if sys.platform == "darwin" else "haqi-native-runtime"))
    parser.add_argument("--asset-manifest", type=Path, help="Explicit replacement asset manifest")
    parser.add_argument("--exe", type=Path, default=os.environ.get("PARAENGINE_EXE"))
    parser.add_argument("--renderer", default="opengl" if sys.platform == "darwin" else "auto")
    parser.add_argument("--prepare-only", action="store_true", help="Refresh files without launching the client")
    parser.add_argument("--manual-login", action="store_true",
                        help="Show the normal Haqi sign-in and selection screens")
    parser.add_argument("--profile-unlimited-fps", action="store_true",
                        help="Request the fastest supported frame rate for this profiling session")
    args = parser.parse_args()
    try:
        runtime = args.runtime_dir.resolve()
        exe = None if args.prepare_only else (args.exe.resolve() if args.exe else find_executable())
        if exe and not exe.is_file():
            raise ValueError(f"Missing executable: {exe}")
        prepare_runtime(args.web_root.resolve(), args.paraworld_root.resolve(), runtime,
                        args.asset_manifest.resolve() if args.asset_manifest else None)
        if args.prepare_only:
            return
        if sys.platform == "darwin":
            bundle = exe.parents[2]
            if bundle.suffix == ".app" and subprocess.run(
                    ["codesign", "--verify", str(bundle)], capture_output=True).returncode:
                raise ValueError(f"Invalid app signature. Rebuild or restore {bundle} before launching.")
        env = dict(os.environ, PARA_ASSET_PATH=runtime.as_posix() + "/")
        command = [str(exe), "--renderer=" + args.renderer,
                   "dev=" + runtime.as_posix() + "/", "writablepath=" + runtime.as_posix() + "/",
                   "logfile=" + (runtime / "log.txt").as_posix(),
                   "noupdate=true", "isDevEnv=true", "debug=main", "mc=false",
                   "bootstrapper=script/apps/Aries/main_loop.lua",
                   "config=config/TaoMee.GameClient.config.xml"]
        if not args.manual_login:
            command.append("haqi_quick_login=true")
        if args.profile_unlimited_fps:
            command.append("profile_unlimited_fps=true")
        with (runtime / "stdout.log").open("a") as output:
            process = subprocess.Popen(command, cwd=runtime, env=env, stdout=output,
                                       stderr=subprocess.STDOUT, start_new_session=True)
        (runtime / "launch.pid").write_text(str(process.pid) + "\n")
        print(f"Launched {exe} (PID {process.pid}); log: {runtime / 'log.txt'}")
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    main()
