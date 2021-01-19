# MLBrickBreaker
AI learns to play brick breaker

## Building

### Repo init

```
git clone git@github.com:hhyyrylainen/MLBrickBreaker.git
cd MLBrickBreaker
git submodule update --init --recursive
```

You need to run the last line above whenever you pull changes that
change the submodule versions.

### Build tools

```sh
pip install scons --user
```

And also install cmake. On Windows make sure it is added to PATH.

### Compiler

On Linux, just install gcc or clang or whatever your systems default compiler is.

On Windows, probably use latest Visual Studio.

### Godot

Install Godot **3.2.3** and create a symbolic link in your PATH to it named `godot`,
so that running:

```sh
$ godot --version
3.2.3.stable.mono.official
```

gives output like that (here the mono version is used but that is not
required for this project).


### Building gdnative library

Commands to finish gdnative setup (replace ALL_CAPITAL with
right values for your system, refer to:
https://docs.godotengine.org/en/3.2/tutorials/plugins/gdnative/gdnative-cpp-example.html
also replace the number after `-j` with your number of CPU threads):

```sh
mkdir build
godot --gdnative-generate-json-api build/api.json
cd third_party/godot-cpp
scons platform=PLATFORM generate_bindings=yes -j20 use_custom_api_file=yes custom_api_file=../../build/api.json bits=64 target=release
# Optional if you want build with debug symbols
scons platform=PLATFORM generate_bindings=yes -j20 use_custom_api_file=yes custom_api_file=../../build/api.json bits=64 target=debug
cd ../..
```

### Tensorflow

First grab the library (with GPU support) for your platform from:
https://www.tensorflow.org/install/lang_c

and unextract it to `third_party/tensorflow` after that you should
have: `third_party/tensorflow/lib` as a folder that contains the
libraries for tensorflow (and a few other files and folders).


### Compile

It's finally time to compile the game modules. 
Configure build:

```sh
cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

For debug build set the type to `Debug`.


For Windows (if you use visual studio), you need to use this instead:
```sh
cmake .. -G "Visual Studio 16 2019" -A x64
```


These steps need to be repeated each time the C++ source code is modified
(note that you need to run this in the build folder):

```sh
cmake --build . --config RelWithDebInfo
```

Alternatively use `make` or visual studio to compile.

### Godot tab config

You need to change "Editor Settings>Text Editor>Indent>Type" to spaces.

## Debugging

To run the debug build, add `.debug` suffixes in `bin/mlbb.gdnlib`. For example on Linux:
```
X11.64="res://bin/x11/libmlbb.debug.so"
```



## Some random notes

GDNative tutorials:
- https://docs.godotengine.org/en/3.2/tutorials/plugins/gdnative/gdnative-cpp-example.html
- https://docs.godotengine.org/en/3.2/tutorials/plugins/gdnative/gdnative-c-example.html#doc-gdnative-c-example
- https://github.com/godotengine/godot-cpp/tree/master
- https://github.com/BastiaanOlij/gdnative_cpp_example
- Might be useful to get Windows build working: https://gist.github.com/willnationsdev/437eeaeea2e675c0bea53343f7ecd4cf

`reloadable=true` needs to be in any GDNative modules to make
recompiling them reload them in the editor, otherwise you will be
closing and reopening the Godot editor a lot.


Tensorflow resources:
- https://github.com/serizba/cppflow/tree/cppflow2
- https://github.com/Neargye/hello_tf_c_api


For export from godot to work, the libs in
`third_party/tensorflow/lib` probably need to be added to
`bin/mlbb.gdnlib`.
