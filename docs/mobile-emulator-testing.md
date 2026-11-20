# Build and test Paracraft on Android and iOS

Use this guide to install the standalone Paracraft app and test it on an Apple Silicon Mac. The Android reference run used an Android 14 (API 34) ARM64 emulator and the Palaka debug APK. The iOS reference run used Xcode 27.2, an iPhone 17 simulator running iOS 27.2, and an ARM64 Release app. A useful smoke result includes the rendered screen, an input that changed it, and the result of returning from the background.

**For a repeat build**, keep the Boost stages, simulator prefix, Gradle cache and Xcode build directory. With the same environment, rerun the Gradle command for Android or the `xcodebuild` command for iOS, reinstall, and repeat the smoke test. Rerun CMake when changing the iOS toolchain or dependency paths. The dependency recipes are for initial setup or a change of target, SDK or library version.

## Shared setup

Start in the checkout root. Run the blocks for your platform in order in the same Bash shell. Keep build products, SDKs and screenshots local. Both platforms need the **contents of a Paracraft client asset directory**. The installed Mac client supplies the packages, scripts, fonts and world templates used in these tests.

```bash
repo="$(git rev-parse --show-toplevel)"
cd "$repo"
qa="$repo/build/mobile-qa"
ios="$qa/ios"
mkdir -p "$qa" "$ios"
export PARACRAFT_ASSETS="${PARACRAFT_ASSETS:-/Applications/Paracraft.app/Contents/Resources/assets}"
export BOOST_ROOT="${BOOST_ROOT:-$repo/../deps/boost_1_85_0}"
test -s "$PARACRAFT_ASSETS/main.pkg" && test -s "$PARACRAFT_ASSETS/main_mobile_res.pkg"
```

Set `PARACRAFT_ASSETS` to another complete client asset tree if needed. `BOOST_ROOT` is the absolute path to the Boost source tree with the `boost/` headers. CMake looks under that tree for a different static library directory for each target. On a new machine, install the SDK or Xcode described below, then prepare the libraries for your target. [Mobile dependency build recipes](mobile-dependencies.md)

## Android

### Build the ARM64 debug APK

Install JDK 17 and the Android SDK command-line tools. Set `JAVA_HOME` to the JDK's home directory (on a Mac with a registered JDK, use `export JAVA_HOME="$(/usr/libexec/java_home -v 17)"`). The SDK packages below match the Gradle project and test image. The app compiles against API 31. This emulator build sets its minimum to API 28 for Boost.Locale's Android iconv support.

```bash
export ANDROID_HOME="${ANDROID_HOME:-$HOME/Library/Android/sdk}"
export PATH="$JAVA_HOME/bin:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools:$ANDROID_HOME/emulator:$PATH"
sdkmanager --install 'platforms;android-31' 'build-tools;34.0.0' 'platform-tools' \
  'ndk;23.1.7779620' 'cmake;3.31.6' 'emulator' 'system-images;android-34;default;arm64-v8a'
test -s "$BOOST_ROOT/android-build/stage/arm64-v8a/lib/libboost_locale.a"
./build_linux.sh android --init-script "$repo/tests/mobile/android-emulator.gradle" \
  -PPROP_MIN_SDK_VERSION=28 -PnplruntimeOce=OFF :Paracraft:assemblePalakaArm64V8aDebug

apk="$repo/NPLRuntime/Platform/AndroidStudio/app/build/outputs/apk/palakaArm64V8a/debug/Paracraft-palaka-arm64V8a-debug.apk"
unzip -Z1 "$apk" | grep -Ex 'lib/arm64-v8a/(libParacraftd|libc\+\+_shared)\.so|assets/(main|main_mobile_res)\.pkg'
```

Expect four entries. The init script adds the external assets to the debug source set and asks Gradle to build the native `Paracraft` target. Gradle invokes the NDK and packages the resulting `libParacraftd.so`. `-PnplruntimeOce=OFF` is for a checkout without the optional CAD sources. Omit it when testing CAD with those sources installed. [Emulator Gradle settings](../tests/mobile/android-emulator.gradle)

### Install, cold launch and test touch

Choose an ARM64 emulator from `adb devices -l`. To create one, run the following once, then start it in a separate terminal using the same SDK environment. On this architecture the APK requires an ARM64 system image.

```bash
echo no | avdmanager create avd --name ParacraftQA --package 'system-images;android-34;default;arm64-v8a'
emulator -avd ParacraftQA
```

For a headless test, start the emulator with `-no-window` and use ADB screenshots and touch input. If an existing AVD is stored outside the default Android directory, export `ANDROID_AVD_HOME` to the directory containing its `.ini` file before running `emulator -list-avds`.

Set `ANDROID_SERIAL` from the device list. Wait for the launcher before installing. The **package** is `com.palaka.paracraft` and its **activity class** is in the `com.tatfook.paracraft` namespace.

```bash
adb devices -l
export ANDROID_SERIAL=emulator-5554  # replace with the selected serial
adb wait-for-device
adb shell getprop sys.boot_completed   # expect 1
adb shell getprop ro.product.cpu.abi   # expect arm64-v8a
adb install -r "$apk"
package=com.palaka.paracraft
activity="$package/com.tatfook.paracraft.ParaEngineActivity"
adb shell am start -S -W -n "$activity"
```

Wait for the first-run agreement or login screen to render. On a fresh install, open the agreement link, verify the content appears, then close it and check that the first screen returns. On an initialized install, open local mode from the login screen. Capture a screenshot after a visible interaction. Use the emulator window or `adb shell input tap X Y` with coordinates from the current screenshot.

```bash
pid="$(adb shell pidof -s "$package" | tr -d '\r')"
test -n "$pid"
adb exec-out screencap -p > "$qa/android-touch.png"
adb shell input keyevent KEYCODE_HOME
# Wait until the Android home screen is visible.
adb shell am start -W -n "$activity"
test "$pid" = "$(adb shell pidof -s "$package" | tr -d '\r')"
# Wait until the app is visible again, then capture it.
adb exec-out screencap -p > "$qa/android-resume.png"
adb logcat -d --pid="$pid" -s ParaEngine:I | grep -E 'JNI_OnLoad|GL_VERSION'
```

Pass when the expected UI returns and responds to touch, the process ID is unchanged, and the log shows JNI initialization and OpenGL ES. An Android “isn’t responding” dialog fails the smoke test even if the PID matches. Inspect `adb logcat -b events -d | grep am_anr` for the package, PID and timestamp of this run. The reference run reported OpenGL ES 3.1. For a process exit, inspect `adb logcat -b crash -d`. For a blank or stalled startup, check the four APK entries above before investigating rendering.

## iOS Simulator

### Build and install the standalone app

Install full Xcode, complete its first-launch setup, and install an iOS Simulator runtime in Xcode Settings → Components or with `xcodebuild -downloadPlatform iOS`. Put CMake 3.31 on PATH (3.31.8 was used here). A phone and a simulator can both be ARM64, but the linker checks the Apple platform of each archive. Prepare the simulator archives once under `$ios/prefix` and the simulator Boost stage under `$BOOST_ROOT`. [Mobile dependency build recipes](mobile-dependencies.md#ios-simulator)

```bash
export DEVELOPER_DIR="${DEVELOPER_DIR:-/Applications/Xcode.app/Contents/Developer}"
xcodebuild -version
xcrun --sdk iphonesimulator --show-sdk-version
xcrun simctl list devices available
mkdir -p NPLRuntime/Platform/iOS/assets
rsync -a "$PARACRAFT_ASSETS/" NPLRuntime/Platform/iOS/assets/

cmake -S NPLRuntime -B "$ios/client" -G Xcode \
  -DCMAKE_TOOLCHAIN_FILE="$repo/NPLRuntime/cmake/ios.toolchain.cmake" \
  -DIOS_PLATFORM=SIMULATOR -DARCHS=arm64 -DDEPLOYMENT_TARGET=16.0 -DENABLE_BITCODE=OFF \
  -DBOOST_ROOT="$BOOST_ROOT" -DBoost_NO_BOOST_CMAKE=ON \
  -DSIMULATOR_PREFIX:PATH="$ios/prefix" -DCMAKE_PROJECT_INCLUDE="$repo/tests/mobile/ios-simulator.cmake" \
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO
xcodebuild -project "$ios/client/NPLRuntime.xcodeproj" -target Paracraft \
  -configuration Release -sdk iphonesimulator -arch arm64 -jobs 8 \
  IPHONEOS_DEPLOYMENT_TARGET=16.0 CODE_SIGNING_ALLOWED=YES CODE_SIGN_IDENTITY=- \
  ONLY_ACTIVE_ARCH=YES DEBUG_INFORMATION_FORMAT=dwarf COMPILER_INDEX_STORE_ENABLE=NO build

app="$ios/client/bin/Release/Paracraft.app"
test -s "$app/assets/main.pkg"
codesign --verify "$app"
xcrun vtool -show-build "$app/Paracraft"    # expect platform IOSSIMULATOR
```

These commands build the executable and sign the simulator bundle ad hoc. The [simulator CMake include](../tests/mobile/ios-simulator.cmake) selects the rebuilt archives for this build. The root `./build_linux.sh ios sim` command only generates an Xcode project. For physical devices or a framework build, use the separate device and framework entries in the [build matrix](../README.md).

### Create a local world and test resume

Choose an iPhone UDID from `xcrun simctl list devices available`. If the runtime has no devices, create an iPhone in Xcode's device manager first. Open the simulator display from Xcode (Device Hub in Xcode 27.2).

```bash
udid='replace-with-selected-simulator-UDID'
bundle=com.tatfook.paracraft
xcrun simctl bootstatus "$udid" -b
xcrun simctl install "$udid" "$app"
launch="$(xcrun simctl launch --terminate-running-process \
  --stdout="$ios/smoke-stdout.log" --stderr="$ios/smoke-stderr.log" "$udid" "$bundle")"
printf '%s\n' "$launch"    # bundle ID followed by process ID
```

1. Wait for the login screen and dismiss the tracking prompt if it appears. Tap **本地模式** (Local mode), then **新建项目** (New project).
2. Choose **小型平坦世界** (small flat world). Enter a new name, for example `SimulatorSmoke-2`, and tap **创建新世界**. Expect a 3D character on grass, the joystick and the block hotbar. This path requires no account.
3. Capture the world, put Paracraft in the background by opening Settings, then bring it back. Wait until Settings is visible before returning.

```bash
xcrun simctl io "$udid" screenshot "$ios/ios-world.png"
xcrun simctl launch "$udid" com.apple.Preferences
# After Settings appears:
resumed="$(xcrun simctl launch "$udid" "$bundle")"
test "$resumed" = "$launch"
# Wait for the same world to be visible again.
xcrun simctl io "$udid" screenshot "$ios/ios-resume.png"
grep -m1 'ParaRHI initialized backend: Metal' "$ios/smoke-stdout.log"
```

Pass when both screenshots show the same world, Metal initialized, and the resumed process ID matches the cold launch. If the linker reports “building for iOS Simulator” while linking an iOS object, check the simulator prefix, Boost stage, and the paths in `$ios/client/CMakeCache.txt`. For an early process exit, inspect the captured stderr and the simulator's system log with `xcrun simctl spawn "$udid" log show --last 5m --predicate 'process == "Paracraft"'`.
