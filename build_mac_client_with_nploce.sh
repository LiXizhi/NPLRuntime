pushd .

if [[ ! -d "NPLRuntime/Plugins/NplOce/NplOce" ]]; then 
	echo "clone NplOce!!!!!"
	git submodule add https://github.com/zhangleio/NplOce.git NPLRuntime/Plugins/NplOce/NplOce
fi


if [[ ! -d "NPLRuntime/Plugins/NplOce/NplOce-Deps" ]]; then 
	echo "clone NplOce-Deps!!!!"
	git submodule add https://github.com/zhangleio/NplOce-Deps.git NPLRuntime/Plugins/NplOce/NplOce-Deps
fi

git submodule init
git submodule update

mkdir -p ./build/mac/
cd build/mac
cmake -G "Xcode" -DNPLRUNTIME_RENDERER=OPENGL -DMAC_SERVER=OFF -DNPLRUNTIME_OCE=TRUE ../../NPLRuntime
popd
