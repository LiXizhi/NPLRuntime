

pushd .
mkdir -p ./prebuild/apple
cd prebuild/apple
sh ../../scripts/boost.sh -ios -macos  --no-framework --boost-version 1.73.0 --boost-libs "thread date_time filesystem system chrono regex serialization iostreams log"
popd
