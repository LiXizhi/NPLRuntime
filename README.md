[![Build Status](https://travis-ci.org/LiXizhi/NPLRuntime.svg?branch=master)](https://travis-ci.org/LiXizhi/NPLRuntime)

## NPL Runtime
NPL runtime allows you to run NPL/lua script from anywhere. such as
``` 
	npls hello.lua
```

## Build From Source
 - To install on linux, run `./build_linux.sh`. See also `.travis.yml` for installing dependencies.
 - To install on windows, [see here](https://github.com/LiXizhi/ParaCraftSDK/wiki/InstallNPLRuntime)  
 - The final output is installed in `./ParaWorld/bin64` folder
 
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