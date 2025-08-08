#!/bin/bash

# iOS Framework Build Script for NPLRuntime
# This script builds NPLRuntime as an iOS Framework

set -e

# Configuration
BUILD_DIR="build_ios_framework"
INSTALL_DIR="install_ios_framework"
CMAKE_TOOLCHAIN="cmake/ios.toolchain.cmake"
NPLRUNTIME_DIR="NPLRuntime"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building NPLRuntime iOS Framework...${NC}"

# Check if NPLRuntime directory exists
if [ ! -d "$NPLRUNTIME_DIR" ]; then
    echo -e "${RED}✗ NPLRuntime directory not found${NC}"
    echo -e "${YELLOW}Please run this script from the project root directory${NC}"
    exit 1
fi

# Clean previous build
if [ -d "$NPLRUNTIME_DIR/$BUILD_DIR" ]; then
    echo -e "${YELLOW}Cleaning previous build...${NC}"
    rm -rf "$NPLRUNTIME_DIR/$BUILD_DIR"
fi

if [ -d "$NPLRUNTIME_DIR/$INSTALL_DIR" ]; then
    echo -e "${YELLOW}Cleaning previous install...${NC}"
    rm -rf "$NPLRUNTIME_DIR/$INSTALL_DIR"
fi

# Change to NPLRuntime directory and create build directory
cd "$NPLRUNTIME_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo -e "${YELLOW}Configuring CMake for iOS Framework...${NC}"

# Set Boost paths for iOS
BOOST_IOS_PREFIX="$PWD/../externals/boost/prebuild/apple/build/boost/1.84.0/ios/prefix"
export BOOST_ROOT="$BOOST_IOS_PREFIX"

# Create necessary directories and copy Info.plist files for iOS build
mkdir -p builds/mac
if [ -f "../externals/freetype-2.8.1/builds/mac/freetype-Info.plist" ]; then
    cp "../externals/freetype-2.8.1/builds/mac/freetype-Info.plist" builds/mac/
fi

# Restore module.modulemap if it was previously backed up
if [ -f "../Platform/iOS/module.modulemap.backup" ] && [ ! -f "../Platform/iOS/module.modulemap" ]; then
    echo -e "${YELLOW}Restoring module.modulemap for CMake configuration...${NC}"
    cp "../Platform/iOS/module.modulemap.backup" "../Platform/iOS/module.modulemap"
fi

# Configure CMake for iOS with Framework enabled
cmake .. \
    -G Xcode \
    -DCMAKE_TOOLCHAIN_FILE="../${CMAKE_TOOLCHAIN}" \
    -DPLATFORM=OS64 \
    -DDEPLOYMENT_TARGET=14.0 \
    -DCMAKE_INSTALL_PREFIX="../${INSTALL_DIR}" \
    -DBUILD_FRAMEWORK=ON \
    -DIOS=ON \
    -DNPLRUNTIME_STATIC_LIB=ON \
    -DNPLRUNTIME_LUAJIT21=ON \
    -DNPLRUNTIME_SUPPORT_FBX=ON \
    -DNPLRUNTIME_PHYSICS=ON \
    -DNPLRUNTIME_AUDIO=ON \
    -DCMAKE_XCODE_ATTRIBUTE_SKIP_INSTALL=NO \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED=NO \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY="" \
    -DCMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE=NO

echo -e "${YELLOW}Building Framework with xcodebuild...${NC}"

# Temporarily remove module.modulemap from Xcode project to avoid conflicts
if [ -f "NPLRuntime.xcodeproj/project.pbxproj" ]; then
    echo -e "${YELLOW}Removing module.modulemap references from Xcode project...${NC}"
    # Create a backup of the project file
    cp "NPLRuntime.xcodeproj/project.pbxproj" "NPLRuntime.xcodeproj/project.pbxproj.backup"
    
    # Remove module.modulemap references from the Xcode project
    sed -i '' '/module\.modulemap/d' "NPLRuntime.xcodeproj/project.pbxproj"
fi

# Build the Framework using xcodebuild
xcodebuild -project NPLRuntime.xcodeproj \
    -target ParacraftFramework \
    -configuration Release \
    -sdk iphoneos \
    CODE_SIGNING_REQUIRED=NO \
    CODE_SIGN_IDENTITY="" \
    ENABLE_BITCODE=NO \
    ONLY_ACTIVE_ARCH=NO \
    -allowProvisioningUpdates

# Restore the Xcode project file
if [ -f "NPLRuntime.xcodeproj/project.pbxproj.backup" ]; then
    echo -e "${YELLOW}Restoring Xcode project file...${NC}"
    mv "NPLRuntime.xcodeproj/project.pbxproj.backup" "NPLRuntime.xcodeproj/project.pbxproj"
fi

# Restore the original module.modulemap if it was backed up and copy to framework
if [ -f "../Platform/iOS/module.modulemap.backup" ]; then
    echo -e "${YELLOW}Restoring original module.modulemap...${NC}"
    cp "../Platform/iOS/module.modulemap.backup" "../Platform/iOS/module.modulemap"
    
    # Check where the framework was actually created and copy our module.modulemap there
    FRAMEWORK_PATHS=(
        "./lib/Release/ParacraftFramework.framework"
        "./build/Release-iphoneos/ParacraftFramework.framework"
        "./Release-iphoneos/ParacraftFramework.framework"
        "./build/ParacraftFramework.build/Release-iphoneos/ParacraftFramework.framework"
    )
    
    for FRAMEWORK_PATH in "${FRAMEWORK_PATHS[@]}"; do
        if [ -d "$FRAMEWORK_PATH" ]; then
            echo -e "${YELLOW}Found framework at: ${FRAMEWORK_PATH}${NC}"
            mkdir -p "$FRAMEWORK_PATH/Modules"
            cp "../Platform/iOS/module.modulemap" "$FRAMEWORK_PATH/Modules/"
            echo -e "${GREEN}Copied module.modulemap to framework${NC}"
            break
        fi
    done
fi

echo -e "${GREEN}Framework build completed!${NC}"

# Check if Framework was created in any of the possible locations
FRAMEWORK_PATHS=(
    "./lib/Release/ParacraftFramework.framework"
    "./build/Release-iphoneos/ParacraftFramework.framework"
    "./Release-iphoneos/ParacraftFramework.framework"
    "./build/ParacraftFramework.build/Release-iphoneos/ParacraftFramework.framework"
)

FRAMEWORK_FOUND=false
FOUND_FRAMEWORK=""

for FRAMEWORK_PATH in "${FRAMEWORK_PATHS[@]}"; do
    if [ -d "$FRAMEWORK_PATH" ]; then
        echo -e "${GREEN}✓ Framework created at: ${FRAMEWORK_PATH}${NC}"
        FRAMEWORK_FOUND=true
        FOUND_FRAMEWORK="$FRAMEWORK_PATH"
        
        # Copy to standard location if not already there
        if [ "$FRAMEWORK_PATH" != "./lib/Release/ParacraftFramework.framework" ]; then
            mkdir -p "./lib/Release"
            cp -R "$FRAMEWORK_PATH" "./lib/Release/"
            echo -e "${GREEN}Copied framework to standard location: ./lib/Release/ParacraftFramework.framework${NC}"
        fi
        
        # Display Framework info
        echo -e "${YELLOW}Framework Info:${NC}"
        ls -la "$FRAMEWORK_PATH"
        
        # Show Framework structure
        echo -e "${YELLOW}Framework Structure:${NC}"
        find "$FRAMEWORK_PATH" -type f | head -20
        break
    fi
done

if [ "$FRAMEWORK_FOUND" = false ]; then
    echo -e "${RED}✗ Framework not found at expected locations${NC}"
    echo -e "${YELLOW}Checking build output:${NC}"
    find . -name "*.framework" -type d 2>/dev/null || echo "No .framework directories found"
fi

echo -e "${GREEN}Build script completed!${NC}"

# Usage instructions
echo -e "${YELLOW}"
echo "Usage Instructions:"
echo "==================="
echo "1. The Framework will be located in: $NPLRUNTIME_DIR/$BUILD_DIR/lib/Release/ParacraftFramework.framework"
echo "2. To use in your iOS project:"
echo "   - Drag ParacraftFramework.framework into your Xcode project"
echo "   - Add it to 'Embedded Frameworks' in your target settings"
echo "   - Import with: #import <ParacraftFramework/ParacraftFramework.h>"
echo "3. To build different configurations:"
echo "   - Debug: cd $NPLRUNTIME_DIR/$BUILD_DIR && xcodebuild -project NPLRuntime.xcodeproj -target ParacraftFramework -configuration Debug"
echo "   - Release: cd $NPLRUNTIME_DIR/$BUILD_DIR && xcodebuild -project NPLRuntime.xcodeproj -target ParacraftFramework -configuration Release"
echo -e "${NC}"
