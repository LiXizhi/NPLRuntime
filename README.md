## NPL Runtime
NPL runtime allows you to run NPL/lua script from anywhere. such as
``` 
	npls hello.lua
```

## Build From Source
run cmake from root directory. On linux, one can also run `./build_linux.sh`.
	
## Install on Win32
run `install.bat` to automatically add NPL runtime folder to environment path. 
One can also manually add the `NPLRuntime/win/bin` folder to system environment path, 
in case you want to run npl script as a system service, such as from hudson CI server.

## Usage
To run with GUI, use:
``` 
    npl [parameters...]
```    
To run in server mode, use:
```	
	npls [filename] [parameters...]
```    
For example:
```	
	npls hello.lua
```    