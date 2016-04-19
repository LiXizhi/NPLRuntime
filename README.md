[![Build Status](https://travis-ci.org/LiXizhi/NPLRuntime.svg?branch=master)](https://travis-ci.org/LiXizhi/NPLRuntime)
[![Code docs](https://codedocs.xyz/LiXizhi/NPLRuntime.svg)](https://codedocs.xyz/LiXizhi/NPLRuntime)


Welcome to the NPL Runtime!
-------------------------
NPL or Neural Parallel Language is an open source, high-performance, scripting language. Its syntax is 100%-compatible with [lua](http://www.lua.org). NPL runtime provides essential functionality for building `3D/2D/Server` applications that runs on `windows/linux/android/iOS`. It is similar to [java](www.java.com), but is more flexible in nature.

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
local function activate()
   if(msg) then
      print(msg.data or "");
   end
   NPL.activate("(gl)helloworld.lua", {data="hello world!"})
end
NPL.this(activate); 
```

### Why a New Programming Language?
NPL prototype was designed in 2004, which was then called 'parallel oriented language'. NPL is initially designed to write flexible algorithms that works in a multi-threaded, and distributed environment with many computers across the network. More specifically, I want to have a language that is suitable for writing neural network algorithms, 3d simulation and visualization. Lua and C/C++ affinity was chosen from the beginning. 

 
### Usage
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
