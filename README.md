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


### First time build commands

Commands for setting up remainder of things (replace ALL_CAPITAL with
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

### Compile

It's finally time to compile the game modules. 
Configure build:

```sh
cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

For debug build set the type to `Debug`.

These steps need to be repeated each time the C++ source code is modified:

```sh
cmake --build .
```

Alternatively use `make` or visual studio to compile.
