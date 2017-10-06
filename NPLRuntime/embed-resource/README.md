# Embed Resource
- modified from https://github.com/cyrilcode/embed-resource

Embed binary files and resources (such as GLSL Shader source files) into
C++ projects. Uses C++11 features and Boost for filesystem.

Include this repository in your CMake based project:

    git submodule add https://github.com/tatfook/embed-resource.git lib/embed-resource

Then add to your CMakeLists.txt for your project:

    include_directories(lib/embed-resource)
    add_subdirectory(lib/embed-resource)

Now you can add your resources, by calling the provided `embed_resources()` or `embed_resources_abs()` function in your
CMakeLists.txt file:

    embed_resources(MyResources shaders/vertex.glsl shaders/frag.glsl)
	embed_resources_abs(MyResources absolute_filename)


Then link to your binary by adding the created variable to your add_executable statement:

    add_executable(MyApp ${SOURCE_FILES} ${MyResources})

In your C++ project you can access your embed resources using the Resource class
provided in `Resource.h`. Here's an example:

    #include <iostream>
    using namespace std;

    #include "embedresource.h"

    int main() {

        ParaEngine::Resource text = LOAD_RESOURCE(frag_glsl);
        cout << string(text.data(), text.size()) << endl;

        return EXIT_SUCCESS;
    }

NB: To reference the file, replace the `.` in `frag.glsl` with an underscore `_`.
So, in this example, the symbol name is `frag_glsl`.

### Credits...

This uses ideas from
[this blog post](https://beesbuzz.biz/blog/e/2014/07/31-embedding_binary_resources_with_cmake_and_c11.php)
which included the Resource class and macro idea. Unfortunately, the use of `ld` command didn't work on OSX.
So, instead, I've used a simple C++ executable that does the same conversion, based on ideas from
[this Stack Overflow question](http://stackoverflow.com/questions/11813271/embed-resources-eg-shader-code-images-into-executable-library-with-cmake).

## License

Public Domain / [WTFPL](http://www.wtfpl.net/)
