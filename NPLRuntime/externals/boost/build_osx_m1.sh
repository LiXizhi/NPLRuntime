#!/bin/bash

pushd .

mkdir -p ./prebuild/apple
cd prebuild/apple

sh ../../build_boost_apple.sh \
-macos -m1 --no-framework --boost-version 1.78.0 \
--boost-libs "thread date_time filesystem system chrono regex serialization iostreams log locale"

popd
