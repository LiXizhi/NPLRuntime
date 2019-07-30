# coding=utf-8

import os;
import sys;
import platform;
import argparse;
import shutil;
import urllib;
import tarfile;
import zipfile;
import socket;


CURRENT_DIR			= os.getcwd();

BOOST_SRC			= os.getenv("BOOST_ROOT");

TOOLCHAIN_DIR		= os.path.join(CURRENT_DIR, "prebuild", "android", "toolchain");
ANDROID_LOG_PATH	= os.path.join(CURRENT_DIR, "prebuild", "android", "android-build.log");

ndk_path 			= "";

def doneSection():
	print "\nDone\n=================================================================\n\n";
	
	
def updateBoostForAndroid(arch):
	print "Updating boost into %s..." %(BOOST_SRC);
	
	jam_path		= os.path.join(BOOST_SRC, "android-config_%s.jam" % (arch));
	
	toolchain_dir 	= os.path.join(TOOLCHAIN_DIR, arch);
	toolchain_dir	= toolchain_dir.replace("\\", "/");
	
	fo = open(jam_path, "w");
	
	if (arch == "armeabi-v7a"):
		fo.write("import os ;\n");
		fo.write("import option ;\n");
		fo.write("import build-system ;\n\n");
		fo.write("option.set layout : system ;\n\n");
		fo.write("modules.poke : NO_BZIP2 : 1 ; \n\n");
		fo.write("using clang : android\n");
		fo.write(":\n");
		fo.write("%s/bin/clang++\n" %(toolchain_dir));
		fo.write(":\n");
		fo.write("<compileflags>-fexceptions\n");
		fo.write("<compileflags>-frtti\n");
		fo.write("<compileflags>-fpic\n");
		fo.write("<compileflags>-ffunction-sections\n");
		fo.write("<compileflags>-funwind-tables\n");
		fo.write("<compileflags>-march=armv7-a\n");
		fo.write("<compileflags>-mfloat-abi=softfp\n");
		fo.write("<compileflags>-mfpu=vfpv3-d16\n");
		fo.write("<compileflags>-fomit-frame-pointer\n");
		fo.write("<compileflags>-fno-strict-aliasing\n");
		fo.write("<compileflags>-I%s/sysroot/usr/include\n" %(toolchain_dir));
		fo.write("<compileflags>--sysroot=%s/sysroot\n" %(toolchain_dir));
		fo.write("<compileflags>-Wa,--noexecstack\n");
		fo.write("<compileflags>-DANDROID\n");
		fo.write("<compileflags>-D__ANDROID__\n");
		fo.write("<compileflags>-DNDEBUG\n");
		fo.write("<compileflags>-O2\n");
		fo.write("<compileflags>-g\n");
		fo.write("<compileflags>-I%s/include/c++/4.9.x\n" %(toolchain_dir));
		fo.write("<compileflags>-I%s/include/c++/4.9.x/arm-linux-androideabi\n" %(toolchain_dir));
		fo.write("<architecture>arm\n");
		fo.write("<compileflags>-fvisibility=hidden\n");
		fo.write("<compileflags>-fvisibility-inlines-hidden\n");
		fo.write("<compileflags>-fdata-sections\n");
		fo.write("<cxxflags>-D__arm__\n");
		fo.write("<archiver>%s/bin/arm-linux-androideabi-ar\n" % (toolchain_dir));
		fo.write("<ranlib>%s/bin/arm-linux-androideabi-ranlib\n" % (toolchain_dir));
		fo.write(";\n");
	elif (arch == "arm64-v8a"):
		fo.write("import os ;\n");
		fo.write("import option ;\n");
		fo.write("import build-system ;\n\n");
		fo.write("option.set layout : system ;\n\n");
		fo.write("modules.poke : NO_BZIP2 : 1 ; \n\n");
		fo.write("using clang : android\n");
		fo.write(":\n");
		fo.write("%s/bin/clang++\n" %(toolchain_dir));
		fo.write(":\n");
		
		fo.write("<compileflags>-fexceptions\n");
		fo.write("<compileflags>-frtti\n");
		fo.write("<compileflags>-fpic\n");
		fo.write("<compileflags>-D_LITTLE_ENDIAN\n");
		#fo.write("<compileflags>-D_POSIX_THREADS=1\n");
		#fo.write("<compileflags>-D_POSIX_SOURCE\n");
		fo.write("<compileflags>-ffunction-sections\n");
		fo.write("<compileflags>-funwind-tables\n");
		fo.write("<compileflags>-fstack-protector\n");
		fo.write("<compileflags>-fno-short-enums\n");
		fo.write("<compileflags>-march=armv8-a\n");
		fo.write("<compileflags>-mtune=cortex-a53\n");
		fo.write("<compileflags>-fomit-frame-pointer\n");
		fo.write("<compileflags>-fno-strict-aliasing\n");
		#fo.write("<compileflags>-finline-limit=64\n");
		fo.write("<compileflags>--sysroot=%s/sysroot\n" %(toolchain_dir));
		fo.write("<compileflags>-Wa,--noexecstack\n");
		fo.write("<compileflags>-target\n");
		fo.write("<compileflags>aarch64-none-linux-android\n");
		fo.write("<compileflags>-DANDROID\n");
		fo.write("<compileflags>-D__ANDROID_API__=21\n");
		fo.write("<compileflags>-D__ANDROID__\n");
		fo.write("<compileflags>-DNDEBUG\n");
		fo.write("<compileflags>-O2\n");
		fo.write("<compileflags>-Os\n");
		fo.write("<compileflags>-Wno-psabi\n");
		fo.write("<compileflags>-g\n");
		fo.write("<compileflags>-I%s/include/c++/4.9.x\n" %(toolchain_dir));
		fo.write("<compileflags>-I%s/include/c++/4.9.x/aarch64-linux-android\n" %(toolchain_dir));
		fo.write("<compileflags>-isystem\n");
		fo.write("<compileflags>%s/sysroot/usr/include/aarch64-linux-android\n" % (ndk_path));
		#fo.write("<compileflags>-D__arm__\n");
		#fo.write("<compileflags>-DARM_NEON\n");
		#fo.write("<compileflags>-mfpu=neon\n");
		#fo.write("<compileflags>-D__aarch64__\n");
		#fo.write("<compileflags>-std=c++11\n");
		fo.write("<architecture>arm\n");
		fo.write("<archiver>%s/bin/aarch64-linux-android-ar\n" % (toolchain_dir));
		fo.write("<ranlib>%s/bin/aarch64-linux-android-ranlib\n" % (toolchain_dir));
		fo.write(";\n");

	fo.close();
	
	doneSection();
	
def downloadCallback(dataCount, dataSize, fileSize):
	per = 100.0 * dataCount * dataSize / fileSize;
	if per > 100:
		per = 100;
		print "%.2f%%\n" % (per);
		
def autoDown(url, name , cb):
	try:
		urllib.urlretrieve(url,name, cb);
	except socket.timeout:
		count = 1;
		while count <= 5:
			try:
				urllib.urlretrieve(url,name, cb);
				break;
			except socket.timeout:
				err_info = 'Reloading for %d time'% count if count == 1 else 'Reloading for %d times' % count;
				print(err_info)
				count += 1;
		if count > 5:
			os.remove(name);
		
def checkSrc(boost_version):
	global BOOST_SRC;
	
	boost_path = "boost_%s" % (boost_version.replace(".", "_"));
	boost_save_path = os.path.join(CURRENT_DIR, "prebuild", "src");
	
	ext_name = "tar.bz2";
	if (platform.system() == "Windows"):
		ext_name = "zip";

	if (BOOST_SRC == None):
		BOOST_SRC 		= os.path.join(boost_save_path, boost_path);

	if (not os.path.exists(BOOST_SRC)):
		
		file_name = "%s.%s" % (boost_path, ext_name);
		file_save_path = os.path.join(boost_save_path, file_name);
		
		if (not os.path.exists(file_save_path)):
			print "Downloading boost %s\n" % (boost_version);

			url = "https://sourceforge.net/projects/boost/files/boost/%s/%s/download" % (boost_version, file_name);
			autoDown(url, file_save_path, downloadCallback);
			
		if (not os.path.exists(file_save_path)):
			print "download failed\n";
			exit(5);
		
		print "untar zip file %s" % (file_save_path);
		
		if (platform.system() == "Windows"):
			zip_file = zipfile.ZipFile(file_save_path);
			zip_file.extractall(boost_save_path);
			zip_file.close();
		else:
			tar = tarfile.open(file_save_path);
			tar.extractall(boost_save_path);
			tar.close();
		
		if (not os.path.exists(BOOST_SRC)):
			print " Not found boost src at %s\n" % (BOOST_SRC);
			exit(1);

def makeToolchain(arch):
	global ndk_path;

	toolchain_dir 	= os.path.join(TOOLCHAIN_DIR, arch)

	if (not os.path.exists(toolchain_dir)):
		print "Making android ndk toolchain\n";
		
		ndk_path = os.getenv("ANDROID_NDK");
		
		if (ndk_path == None):
			ndk_path = os.getenv("NDK_ROOT");
			
		if (ndk_path == None):
			print "Not found env ANDROID_NDK or NDK_ROOT\n"
			exit(2);

		
		make_standalone_toolchain = os.path.join(ndk_path, "build", "tools", "make_standalone_toolchain.py");
		if (not os.path.exists(make_standalone_toolchain)):
			print " Not found make_standalone_toolchain at %s\n" % (make_standalone_toolchain);
			exit(3);
		
		if (arch == "armeabi-v7a"):
			cmd =  "python %s --arch arm --api 21 --stl libc++ --force --install-dir %s" % (make_standalone_toolchain, toolchain_dir);
			os.system(cmd);
		elif (arch == "arm64-v8a"):
			cmd =  "python %s --arch arm64 --api 21 --stl libc++ --force --install-dir %s" % (make_standalone_toolchain, toolchain_dir);
			os.system(cmd);
		
		
def buildAndroid(arch):
	build_dir = "android-build/%s" % (arch);
	stage_dir = "android-build/stage/%s" % (arch);

	makeToolchain(arch);
	updateBoostForAndroid(arch);
	
	address_model = 32;
	if (arch == "arm64-v8a"):
		address_model = 64;

	os.chdir(BOOST_SRC);
	print "Building Boost for Android \n";
	
	params = "--user-config=android-config_%s.jam --build-dir=%s --stagedir=%s toolset=clang-android target-os=android threadapi=pthread threading=multi link=static runtime-link=shared address-model=%d abi=aapcs " % (arch, build_dir, stage_dir, address_model);
	params += "--with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-regex --with-serialization --with-iostreams --with-log";
	
	ret = 0;
	if (platform.system() == "Windows"):
		#os.system("bootstrap --with-toolset=gcc > %s" % (ANDROID_LOG_PATH));
		os.system("bootstrap > %s" % (ANDROID_LOG_PATH));
		#ret = os.system("b2 %s --abbreviate-paths >> %s" % (params, ANDROID_LOG_PATH));
		ret = os.system("b2 %s --abbreviate-paths" % (params));
	else:
		os.system("./bootstrap.sh --with-toolset=gcc >> %s 2>&1" % (ANDROID_LOG_PATH));
		ret = os.system("./b2 %s >> %s 2>&1" % (params, ANDROID_LOG_PATH));
	
	if (ret != 0):
		print "Error staging Android. Check log.\n";
		os.chdir(CURRENT_DIR);
		exit(4);
	
	os.chdir(CURRENT_DIR);
	
	print "Completed successfully\n";
	
def buildWin32(b32):
	os.chdir(BOOST_SRC);

	sub = "x86";
	address_model = 32;
	if (not b32):
		sub = "x64";
		address_model = 64;
		
	build_dir = "build_win32/build/%s" % (sub);
	stage_dir = "build_win32/stage/%s" % (sub);
	
	print "Cleaning everything\n";
	if (os.path.exists(build_dir)):
		shutil.rmtree(build_dir);
	if (os.path.exists(stage_dir)):
		shutil.rmtree(stage_dir);
	
	if (b32):
		print "Building for win32\n";
	else:
		print "Building for win64\n";
		
	os.system("bootstrap --with-toolset=gcc");
	params = "stage link=static runtime-link=static threading=multi address-model=%d --abbreviate-paths cflags=-D_WIN32_WINNT=0x0501 --build-dir=%s --stagedir=%s" % (address_model, build_dir, stage_dir);
	params += " --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-regex --with-serialization --with-iostreams --with-log";
	ret = os.system("b2 %s --abbreviate-paths" % (params));
	
	os.chdir(CURRENT_DIR);
	if (ret == 0):
		print "Completed successfully\n";
	else:
		print "Completed failed\n";
	
def buildApple():
	print "Completed successfully\n";
	
def buildLinux():
	os.chdir(BOOST_SRC);
	os.system("./bootstrap.sh --with-toolset=gcc");
	params = "link=static threading=multi variant=release --build-dir=linux-build --stagedir=linux-build/stage";
	params += " --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-regex --with-serialization --with-iostreams --with-log";
	ret = os.system("./b2 %s" % (params));
	if (ret == 0):
		ret = os.system("./b2 install --build-dir=linux-build --stagedir=linux-build/stage");
	
	os.chdir(CURRENT_DIR);
	
	if (ret == 0):
		print "Completed successfully\n";
	else:
		print "Completed failed\n";

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='manual to this script')
	parser.add_argument('--platform', type=str, default = "android");
	parser.add_argument('--boost_version', type=str, default = "1.69.0");
	
	args = parser.parse_args();
	
	checkSrc(args.boost_version);
	
	if (args.platform == "android"):
		buildAndroid("arm64-v8a");
		buildAndroid("armeabi-v7a");
	elif (args.platform == "win32_x86"):
		buildWin32(True)
	elif (args.platform == "win32_x64"):
		buildWin32(False);
	elif (args.platform == "apple"):
		buildApple();
	elif (args.platform == "linux"):
		buildLinux();
	else:
		print "unknown platform %s" % (args.platform);
	
