[![Build status](https://ci.appveyor.com/api/projects/status/j9xgtpmohp9otwsw?svg=true)](https://ci.appveyor.com/project/LiXizhi/nplruntime)
[![Code docs](https://codedocs.xyz/LiXizhi/NPLRuntime.svg)](https://codedocs.xyz/LiXizhi/NPLRuntime)
[![Documentation Status](https://readthedocs.org/projects/tatfook-npldocs/badge/?version=master)](http://docs.paraengine.com/en/master/?badge=master)


Welcome to the NPL Runtime!
-------------------------
NPL or Neural Parallel Language is an open source, high-performance, scripting language. Its syntax is 100%-compatible with [lua](http://www.lua.org). NPL runtime provides essential functionality for building `3D/2D/Server` applications that runs on `windows/linux/android/iOS`. 

### Install Guide
```
git clone https://github.com/LiXizhi/NPLRuntime.git
./build_linux.sh
```
See [Install Guide](https://github.com/LiXizhi/NPLRuntime/wiki/InstallGuide) for details

### Getting Started
* [What is NPL?](https://github.com/LiXizhi/NPLRuntime/wiki/WhatIsNPL)
* [Tutorial: HelloWorld](https://github.com/LiXizhi/NPLRuntime/wiki/TutorialHelloWorld)
* [Source Code Overview](https://github.com/LiXizhi/NPLRuntime/wiki/SourceCodeOverview)

### Example code
```lua
-- this is from `helloworld.npl`
NPL.activate("(gl)helloworld.npl", {data="hello world!"})
this(msg){
   if(msg) then
      print(msg.data or "");
   end
}
```

### Why a New Programming Language?
NPL prototype was designed in 2004, which was then called 'parallel oriented language'. NPL is initially designed to write flexible algorithms that works in a multi-threaded, and distributed environment with many computers across the network. More specifically, I want to have a language that is suitable for writing neural network algorithms, 3d simulation and visualization. Lua and C/C++ affinity was chosen from the beginning. 

 
### Usage
To run with GUI, use:
``` 
npl [filename] [parameters...]
```    
To run in server mode, use:
```	
npls [filename] [parameters...]
```    
For example:
```	
npls hello.npl
```    
