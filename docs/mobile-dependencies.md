# Cross-compile the mobile test dependencies

Use these recipes once per target when the libraries named in the [emulator build guide](mobile-emulator-testing.md) are missing. Run **Shared setup** from that guide first. The examples use Bash, an Apple Silicon host, and build paths without spaces for the older third-party build tools. Each target has a separate Boost stage. Reuse a populated stage on subsequent app builds.

## Boost source

Both builds used Boost 1.85.0. If you already have its headers and `b2` at `$BOOST_ROOT`, skip this block. Otherwise unpack the source into an empty dependency directory and set `BOOST_ROOT` to the extracted tree. Keep this setting when building the app.

```bash
deps="$qa/deps"
mkdir -p "$deps"
(
  set -e
  cd "$deps"
  curl -fLO https://archives.boost.io/release/1.85.0/source/boost_1_85_0.tar.bz2
  echo '7009fe1faa1697476bdc7027703a2badb84e849b7b0baad5086b087b971f8617  boost_1_85_0.tar.bz2' | shasum -a 256 -c -
  tar xjf boost_1_85_0.tar.bz2
)
export BOOST_ROOT="$deps/boost_1_85_0"
(cd "$BOOST_ROOT" && ./bootstrap.sh)
```

## Android ARM64

Install the NDK version in the [Android build instructions](mobile-emulator-testing.md#build-the-arm64-debug-apk) first. Use the NDK Linux-targeting Clang driver and LLVM archiver. The NDK r23b tool directory on Apple Silicon is named `darwin-x86_64`. Use API 28 for both the compiler driver and the APK. At API 24, libc does not expose the iconv functions used by this Boost.Locale build.

```bash
export ANDROID_HOME="${ANDROID_HOME:-$HOME/Library/Android/sdk}"
llvm="$ANDROID_HOME/ndk/23.1.7779620/toolchains/llvm/prebuilt/darwin-x86_64/bin"
cat > "$qa/android-arm64.jam" <<JAM
using clang-linux : android28 : "$llvm/aarch64-linux-android28-clang++" : <archiver>"$llvm/llvm-ar" <ranlib>"$llvm/llvm-ranlib" ;
JAM
(
  set -e
  cd "$BOOST_ROOT"
  ./b2 -j8 --user-config="$qa/android-arm64.jam" --build-dir="$qa/android-boost-build" \
    --stagedir=android-build/stage/arm64-v8a toolset=clang-linux-android28 \
    target-os=android architecture=arm address-model=64 binary-format=elf \
    cxxstd=17 cxxflags=-fPIC threading=multi link=static runtime-link=shared variant=release \
    --with-thread --with-date_time --with-filesystem --with-chrono --with-regex \
    --with-serialization --with-iostreams --with-log --with-locale --with-atomic --with-system \
    -sNO_ICU=1 -sNO_BZIP2=1 -sNO_ZLIB=1 -sNO_ZSTD=1 -sNO_LZMA=1 stage
)
test -s "$BOOST_ROOT/android-build/stage/arm64-v8a/lib/libboost_locale.a"
```

## iOS Simulator

This recipe targets `arm64-apple-ios16.0-simulator`. Use full Xcode and select its developer directory. Keep scratch sources outside the checkout's tracked external libraries.

```bash
export DEVELOPER_DIR="${DEVELOPER_DIR:-/Applications/Xcode.app/Contents/Developer}"
sdk="$(xcrun --sdk iphonesimulator --show-sdk-path)"
cc="$(xcrun --sdk iphonesimulator --find clang)"
cxx="$(xcrun --sdk iphonesimulator --find clang++)"
flags="-target arm64-apple-ios16.0-simulator -isysroot $sdk"
prefix="$ios/prefix"
src="$ios/src"
mkdir -p "$src" "$prefix/lib"
```

### Boost

The compiler wrapper puts the simulator target and sysroot last so Boost's Darwin toolset uses them for its configuration probes and libraries.

```bash
cat > "$ios/boost-cxx" <<WRAPPER
#!/bin/sh
exec "$cxx" "\$@" -target arm64-apple-ios16.0-simulator -isysroot "$sdk"
WRAPPER
chmod +x "$ios/boost-cxx"
echo "using clang : sim : \"$ios/boost-cxx\" ;" > "$ios/sim-boost.jam"
(
  set -e
  cd "$BOOST_ROOT"
  ./b2 -j8 --user-config="$ios/sim-boost.jam" --build-dir="$ios/boost-build" \
    --stagedir=iphonesim-build/stage toolset=clang-sim target-os=iphone architecture=arm \
    address-model=64 binary-format=mach-o abi=aapcs cxxstd=17 threading=multi \
    link=static runtime-link=shared variant=release --with-thread --with-date_time \
    --with-filesystem --with-chrono --with-regex --with-serialization --with-iostreams \
    --with-log --with-atomic --with-system -sNO_ICU=1 -sNO_BZIP2=1 -sNO_ZLIB=1 \
    -sNO_ZSTD=1 -sNO_LZMA=1 stage
)
```

### LuaJIT, JPEG, OpenSSL and curl

Build the repository's LuaJIT 2.1 and the following upstream source releases to match the engine headers. Use a fresh source directory for the first build. curl needs the simulator OpenSSL headers and libraries, so build it last.

```bash
(
  set -e
  cd "$src"
  curl -fLO https://www.ijg.org/files/jpegsrc.v9.tar.gz
  curl -fLO https://github.com/openssl/openssl/releases/download/OpenSSL_1_1_1w/openssl-1.1.1w.tar.gz
  curl -fLO https://curl.se/download/curl-7.52.1.tar.gz
  shasum -a 256 -c - <<'HASHES'
6e004d72f9108014caccf434095514c139478c72b6b1873a3e3807ea5bec5c59  jpegsrc.v9.tar.gz
cf3098950cb4d853ad95c0841f1f9c6d3dc102dccfcacd521d93925208b76ac8  openssl-1.1.1w.tar.gz
a8984e8b20880b621f61a62d95ff3c0763a3152093a9f9ce4287cfd614add6ae  curl-7.52.1.tar.gz
HASHES
  for archive in jpegsrc.v9.tar.gz openssl-1.1.1w.tar.gz curl-7.52.1.tar.gz; do tar xzf "$archive"; done
  mkdir -p luajit
  cp -R "$repo/NPLRuntime/externals/LuaJIT2_1/code/." luajit/
)
(
  set -e
  cd "$src/luajit"
  make -j6 BUILDMODE=static HOST_CC=clang DEFAULT_CC=clang CROSS="$(dirname "$cc")/" \
    TARGET_SYS=iOS TARGET_FLAGS="$flags" XCFLAGS=-DLUAJIT_ENABLE_GC64
  cp src/libluajit.a "$prefix/lib/"
)
(
  set -e
  cd "$src/jpeg-9"
  CC="$cc" CFLAGS="$flags -O2" ./configure --host=aarch64-apple-darwin --disable-shared --prefix="$prefix"
  make -j6 libjpeg.la
  cp .libs/libjpeg.a "$prefix/lib/"
)
(
  set -e
  cd "$src/openssl-1.1.1w"
  CFLAGS="$flags -O2" LDFLAGS="$flags" ./Configure iossimulator-xcrun no-shared no-tests no-asm --prefix="$prefix"
  make -j6 build_libs
  make install_dev
)
(
  set -e
  cd "$src/curl-7.52.1"
  CC="$cc" CFLAGS="$flags -O2" CPPFLAGS="$flags -I$prefix/include" LDFLAGS="$flags -L$prefix/lib" \
    ./configure --host=aarch64-apple-darwin --disable-shared --enable-static --with-ssl="$prefix" \
    --without-ca-bundle --without-ca-path --without-libidn --without-libssh2 --without-librtmp \
    --without-nghttp2 --disable-ldap --disable-ldaps --prefix="$prefix"
  make -C lib -j6
  cp lib/.libs/libcurl.a "$prefix/lib/"
)
```

The simulator build consumes `libluajit.a`, `libjpeg.a`, `libssl.a`, `libcrypto.a`, `libcurl.a`, and `include/openssl` under `$prefix`. If curl reports missing socket support, check that `CPPFLAGS` includes the simulator target **and** sysroot when running its configure probes. Return to [building and installing the app](mobile-emulator-testing.md#build-and-install-the-standalone-app).
