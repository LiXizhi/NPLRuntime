

pushd .
mkdir -p ./prebuild/apple
cd prebuild/apple
sh ../../scripts/boost.sh -ios -macos  --no-framework --boost-version 1.64.0 --boost-libs "thread date_time filesystem system chrono signals regex serialization iostreams log"
popd