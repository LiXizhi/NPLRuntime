import os
import shutil


ANDROID_SDK = os.environ.get("ANDROID_SDK")
if ANDROID_SDK is None:
    print 'Not find ANDROID_SDK'
    exit()
ANDROID_JDK = os.environ.get("ANDROID_JDK")
if ANDROID_JDK is None:
    print 'Not find ANDROID_JDK'
    exit()
ANDROID_NDK = os.environ.get("ANDROID_NDK")
if ANDROID_NDK is None:
    print 'Not find ANDROID_NDK'
    exit()


ANDROID_NDK = ANDROID_NDK.replace('\\','/')

ANDROID_SYSTEM_VERSION = "19"


print("ANDROID_SDK:" + ANDROID_SDK)
print("ANDROID_JDK:" + ANDROID_JDK)
print("ANDROID_NDK:" + ANDROID_NDK)




def copy_files(sourceDir,  targetDir):
    if(not os.path.exists(targetDir)):
        os.makedirs(targetDir)
    list_dirs = os.walk(sourceDir) 
    for root, dirs, files in list_dirs:   
        for f in files: 
            src_path = os.path.join(root,f)
            dst_path = os.path.join(targetDir,f)
            shutil.copyfile(src_path,dst_path)


# Generate jni
def make_native_lib(abi):
    build_path = "build/Android/native_libs/" + abi
    if not os.path.exists(build_path):
        os.makedirs(build_path)

    os.chdir(build_path)

    build_g = "MinGW Makefiles"
    
    build_arch_abi = abi
    build_stl_type = "gnustl_static"
    build_make_program= ANDROID_NDK + "/prebuilt/windows-x86_64/bin/make.exe"
    build_cmd = "cmake -G \"{0}\" -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION={1} -DCMAKE_ANDROID_ARCH_ABI={2} -DCMAKE_ANDROID_NDK=\"{3}\" -DCMAKE_ANDROID_STL_TYPE={4} -DCMAKE_MAKE_PROGRAM=\"{5}\" ../../../../Client/trunk".format(build_g,ANDROID_SYSTEM_VERSION,build_arch_abi, \
    ANDROID_NDK,build_stl_type,build_make_program)
    os.system(build_cmd)
    os.system("make")
    copy_files("./lib/","../../project/lib/"+abi)

    os.chdir("../../../../")

# Build apk

def build_apk():
    project_path = "./build/Android/project/"
    src_path = "./Client/trunk/ParaEngineClient/Platform/Android/"
    bin_path = project_path + "bin/"
    if os.path.exists(bin_path):
        shutil.rmtree(bin_path)
    os.makedirs(bin_path)

    # Step1 pack res
    # aapt packge -f -M AndroidManifest.xml -I "E:/NVPACK/android-sdk-windows/platforms/android-19/android.jar" -S ./res -F ./bin/res.zip
    cmd = "aapt packge -f -M {0}AndroidManifest.xml -I \"{1}/platforms/android-{2}/android.jar\" -S {0}./res -F {3}bin/res.zip".format(src_path,ANDROID_SDK,ANDROID_SYSTEM_VERSION,project_path)
    os.system(cmd)

    # Step2 
    #java -classpath {SDK}/tools/lib/sdklib.jar com.android.sdklib.build.ApkBuilderMain {BIN}}/bin/test_u.apk -u -z {BIN}}/bin/res.zip -nf {P}/lib
    cmd = "java -classpath {0}/tools/lib/sdklib.jar com.android.sdklib.build.ApkBuilderMain {1}/bin/test_u.apk -u -z {1}/bin/res.zip -nf {1}/lib".format(ANDROID_SDK,project_path) 
    os.system(cmd)

    # Step3 sign
    # jarsigner -keystore {S}/key/test.keystore -storepass 666666 -keypass 666666 -signedjar {P}/test.apk {P}/bin/test_u.apk test.keystore
    cmd = "jarsigner -keystore {0}/key/test.keystore -storepass 666666 -keypass 666666 -signedjar {1}/bin/test.apk {1}/bin/test_u.apk test.keystore".format(src_path,project_path)
    os.system(cmd)

    # Step4 install
    cmd = "adb install -r {0}/bin/test.apk".format(project_path)
    os.system(cmd)
    # Step5 launch

make_native_lib("armeabi")
#make_native_lib("armeabi-v7a")

build_apk()





