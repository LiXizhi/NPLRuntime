#!/usr/bin/env python3
# coding=utf-8
# HarmonyOS Boost Build Script
# Based on Android build script, adapted for HarmonyOS Native development

import os
import platform
import argparse
import shutil
import urllib.request
import tarfile
import socket
import subprocess

CURRENT_DIR = os.getcwd()
BOOST_SRC = os.getenv("BOOST_ROOT")
OHOS_NDK = os.getenv("OHOS_NDK_HOME")  # HarmonyOS NDK path
HARMONYOS_LOG_PATH = os.path.join(CURRENT_DIR, "prebuild", "harmonyos-build.log")

def doneSection():
    print("\nDone\n=================================================================\n\n")

def updateBoostForHarmonyOS(arch):
    """
    Generate Boost configuration file for HarmonyOS
    Similar to Android, but using HarmonyOS Native toolchain
    """
    global OHOS_NDK
    print(f"Updating boost config for HarmonyOS {arch}...")

    jam_path = os.path.join(BOOST_SRC, f"harmonyos-config_{arch}.jam")
    
    # HarmonyOS NDK structure is similar to Android NDK
    # Typically: {OHOS_NDK}/llvm/bin
    toolchain_dir = OHOS_NDK.replace("\\", "/")
    llvm_bin = os.path.join(toolchain_dir, "llvm", "bin")
    
    with open(jam_path, "w") as fo:
        fo.write("import os ;\n")
        fo.write("import option ;\n")
        fo.write("import build-system ;\n\n")
        fo.write("option.set layout : system ;\n\n")
        fo.write("modules.poke : NO_BZIP2 : 1 ; \n\n")
        fo.write("using clang : harmonyos\n")
        fo.write(":\n")
        
        if arch == "armeabi-v7a":
            fo.write(f"{llvm_bin}/clang++\n")
            fo.write(":\n")
            fo.write("<compileflags>-target\n")
            fo.write("<compileflags>arm-linux-ohos\n")
        elif arch == "arm64-v8a":
            fo.write(f"{llvm_bin}/clang++\n")
            fo.write(":\n")
            fo.write("<compileflags>-target\n")
            fo.write("<compileflags>aarch64-linux-ohos\n")
        elif arch == "x86_64":
            fo.write(f"{llvm_bin}/clang++\n")
            fo.write(":\n")
            fo.write("<compileflags>-target\n")
            fo.write("<compileflags>x86_64-linux-ohos\n")
        
        # Common compiler flags
        fo.write("<compileflags>-fexceptions\n")
        fo.write("<compileflags>-frtti\n")
        fo.write("<compileflags>-fpic\n")
        fo.write("<compileflags>-ffunction-sections\n")
        fo.write("<compileflags>-funwind-tables\n")
        fo.write("<compileflags>-fno-strict-aliasing\n")
        fo.write("<compileflags>-DHARMONY_OS\n")
        fo.write("<compileflags>-D__HARMONYOS__\n")
        fo.write("<compileflags>-DNDEBUG\n")
        fo.write("<compileflags>-O2\n")
        fo.write("<compileflags>-g\n")
        fo.write("<compileflags>-fvisibility=hidden\n")
        fo.write("<compileflags>-fvisibility-inlines-hidden\n")
        fo.write("<compileflags>-fdata-sections\n")
        fo.write("<compileflags>-D_LITTLE_ENDIAN\n")
        
        # Archiver and ranlib
        fo.write(f"<archiver>{llvm_bin}/llvm-ar\n")
        fo.write(f"<ranlib>{llvm_bin}/llvm-ranlib\n")
        fo.write(";\n")
    
    doneSection()

def downloadCallback(blocknum, blocksize, totalsize):
    """Download progress callback"""
    readsofar = blocknum * blocksize
    if totalsize > 0:
        percent = readsofar * 1e2 / totalsize
        s = "\r%5.1f%% %*d / %d" % (
            percent, len(str(totalsize)), readsofar, totalsize)
        print(s, end='')
        if readsofar >= totalsize:
            print()
    else:
        print("read %d" % (readsofar,))

def downloadBoost(boost_version):
    """Download Boost source code"""
    boost_path = f"boost_{boost_version.replace('.', '_')}"
    file_name = f"{boost_path}.tar.bz2"
    file_save_path = os.path.join(CURRENT_DIR, file_name)
    
    if not os.path.exists(file_save_path):
        print(f"Downloading boost {boost_version}\n")
        url = f"https://boostorg.jfrog.io/artifactory/main/release/{boost_version}/source/{file_name}"
        
        try:
            urllib.request.urlretrieve(url, file_save_path, downloadCallback)
        except Exception as e:
            print(f"Download failed: {e}\n")
            if os.path.exists(file_save_path):
                os.remove(file_save_path)
            return False
    
    return True

def extractBoost(boost_version):
    """Extract Boost source code"""
    global BOOST_SRC
    
    boost_path = f"boost_{boost_version.replace('.', '_')}"
    file_name = f"{boost_path}.tar.bz2"
    file_save_path = os.path.join(CURRENT_DIR, file_name)
    
    if not os.path.exists(BOOST_SRC) or not os.listdir(BOOST_SRC):
        print(f"Extracting {file_save_path} to {BOOST_SRC}")
        os.makedirs(BOOST_SRC, exist_ok=True)
        
        with tarfile.open(file_save_path, 'r:bz2') as tar:
            # Extract all files
            tar.extractall(BOOST_SRC)
        
        # Move files from boost_X_XX_X to BOOST_SRC
        extracted_dir = os.path.join(BOOST_SRC, boost_path)
        if os.path.exists(extracted_dir):
            for item in os.listdir(extracted_dir):
                src = os.path.join(extracted_dir, item)
                dst = os.path.join(BOOST_SRC, item)
                if os.path.exists(dst):
                    if os.path.isdir(dst):
                        shutil.rmtree(dst)
                    else:
                        os.remove(dst)
                shutil.move(src, dst)
            os.rmdir(extracted_dir)
    
    return True

def bootstrapBoost():
    """Run Boost bootstrap"""
    global BOOST_SRC
    
    bootstrap_script = os.path.join(BOOST_SRC, "bootstrap.sh")
    if not os.path.exists(bootstrap_script):
        print("Error: bootstrap.sh not found!")
        return False
    
    os.chdir(BOOST_SRC)
    print("Running bootstrap.sh...")
    
    ret = os.system("./bootstrap.sh --with-toolset=clang >> %s 2>&1" % HARMONYOS_LOG_PATH)
    os.chdir(CURRENT_DIR)
    
    return ret == 0

def buildHarmonyOS(arch):
    """Build Boost for HarmonyOS architecture"""
    global BOOST_SRC
    
    build_dir = f"harmonyos-build/{arch}"
    stage_dir = f"harmonyos-build/stage/{arch}"
    
    updateBoostForHarmonyOS(arch)
    
    # Determine address model
    address_model = 32
    if arch in ["arm64-v8a", "x86_64"]:
        address_model = 64
    
    os.chdir(BOOST_SRC)
    print(f"Building Boost for HarmonyOS {arch}\n")
    
    # Build parameters
    params = (
        f"--user-config=harmonyos-config_{arch}.jam "
        f"define=BOOST_FILESYSTEM_DISABLE_STATX "
        f"--build-dir={build_dir} "
        f"--stagedir={stage_dir} "
        f"toolset=clang-harmonyos "
        f"target-os=linux "  # HarmonyOS uses Linux-like target
        f"threadapi=pthread "
        f"threading=multi "
        f"link=static "
        f"runtime-link=shared "
        f"address-model={address_model} "
    )
    
    # Add libraries to build
    params += (
        "--with-locale --with-thread --with-date_time --with-filesystem "
        "--with-system --with-chrono --with-regex --with-serialization "
        "--with-iostreams --with-log"
    )
    
    # Run b2
    ret = os.system(f"./b2 {params} >> {HARMONYOS_LOG_PATH} 2>&1")
    
    os.chdir(CURRENT_DIR)
    
    if ret != 0:
        print(f"Error building Boost for {arch}. Check log at {HARMONYOS_LOG_PATH}\n")
        return False
    
    print(f"Successfully built Boost for HarmonyOS {arch}\n")
    return True

def copyLibraries():
    """Copy built libraries to prebuild directory"""
    global BOOST_SRC
    
    prebuild_dir = os.path.join(CURRENT_DIR, "prebuild", "harmonyos")
    os.makedirs(prebuild_dir, exist_ok=True)
    
    # Copy headers
    src_include = os.path.join(BOOST_SRC, "boost")
    dst_include = os.path.join(prebuild_dir, "include", "boost")
    if os.path.exists(src_include):
        if os.path.exists(dst_include):
            shutil.rmtree(dst_include)
        os.makedirs(os.path.dirname(dst_include), exist_ok=True)
        shutil.copytree(src_include, dst_include)
        print(f"Copied headers to {dst_include}")
    
    # Copy libraries for each architecture
    for arch in ["arm64-v8a", "armeabi-v7a"]:
        stage_lib = os.path.join(BOOST_SRC, f"harmonyos-build/stage/{arch}/lib")
        if os.path.exists(stage_lib):
            dst_lib = os.path.join(prebuild_dir, arch)
            os.makedirs(dst_lib, exist_ok=True)
            
            for lib_file in os.listdir(stage_lib):
                if lib_file.endswith('.a'):
                    src = os.path.join(stage_lib, lib_file)
                    dst = os.path.join(dst_lib, lib_file)
                    shutil.copy2(src, dst)
            
            print(f"Copied libraries for {arch} to {dst_lib}")

def main():
    if BOOST_SRC is None:
        print("Error: BOOST_ROOT environment variable not set!")
        print("Please set it to the directory where Boost should be extracted")
        print("Example: export BOOST_ROOT=/path/to/boost")
        exit(1)
    
    if OHOS_NDK is None:
        print("Error: OHOS_NDK_HOME environment variable not set!")
        print("Please set it to your HarmonyOS NDK path")
        print("Example: export OHOS_NDK_HOME=/path/to/harmonyos-sdk/native")
        exit(1)
    
    parser = argparse.ArgumentParser(description='Build Boost for HarmonyOS')
    parser.add_argument('--boost_version', type=str, default="1.78.0",
                       help='Boost version to build (default: 1.78.0)')
    parser.add_argument('--arch', type=str, default="all",
                       choices=['arm64-v8a', 'armeabi-v7a', 'all'],
                       help='Architecture to build (default: all)')
    parser.add_argument('--skip-download', action='store_true',
                       help='Skip downloading Boost (use existing source)')
    
    args = parser.parse_args()
    
    # Create log directory
    os.makedirs(os.path.dirname(HARMONYOS_LOG_PATH), exist_ok=True)
    
    # Download and extract Boost
    if not args.skip_download:
        if not downloadBoost(args.boost_version):
            exit(1)
        if not extractBoost(args.boost_version):
            exit(1)
        if not bootstrapBoost():
            exit(1)
    
    # Build for requested architectures
    success = True
    if args.arch == "all":
        for arch in ["arm64-v8a", "armeabi-v7a"]:
            if not buildHarmonyOS(arch):
                success = False
    else:
        if not buildHarmonyOS(args.arch):
            success = False
    
    if success:
        copyLibraries()
        print("\n" + "="*65)
        print("Boost build completed successfully!")
        print("="*65)
    else:
        print("\n" + "="*65)
        print("Boost build failed! Check the log file:")
        print(HARMONYOS_LOG_PATH)
        print("="*65)
        exit(1)

if __name__ == '__main__':
    main()
