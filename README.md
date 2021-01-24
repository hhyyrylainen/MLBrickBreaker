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


### First time build commands

Commands for setting up remainder of things (replace ALL_CAPITAL with
right values for your system, refer to:
https://docs.godotengine.org/en/3.2/tutorials/plugins/gdnative/gdnative-cpp-example.html
also replace the number after `-j` with your number of CPU threads):

```sh
mkdir build
# Use this if Godot tries to import .obj files from the build folder:
touch build/.gdignore
godot --gdnative-generate-json-api build/api.json
cd third_party/godot-cpp
scons platform=PLATFORM generate_bindings=yes -j20 use_custom_api_file=yes custom_api_file=../../build/api.json bits=64 target=release
# if don't want to build in debug mode this may not be needed:
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


NEAT resources:
- http://nn.cs.utexas.edu/?neat-c
- http://nn.cs.utexas.edu/downloads/papers/stanley.ec02.pdf
- https://en.wikipedia.org/wiki/Neuroevolution
- https://en.wikipedia.org/wiki/Neuroevolution_of_augmenting_topologies
- http://www.cs.ucf.edu/~kstanley/neat.html


## Starting networks

startgenes, commented (when copying to config remove from `#` to line ends):

```
genomestart 1
trait 1 0.1 0 0 0 0 0 0 0
node 1 0 1 1 # input (paddle x)
node 2 0 1 1 # input (ball x)
node 3 0 1 1 # input (ball y)
node 4 0 1 1 # input (lowest brick x)
node 5 0 1 1 # input (lowest brick y)
node 6 0 0 2 # output (left)
node 7 0 0 2 # output (right)
node 8 0 0 2 # output (special)
gene 1 1 6 0.0 0 1 0 1 # link 1->6
gene 1 2 6 0.0 0 1 0 1 # link 2->6
gene 1 3 7 0.0 0 1 0 1 # link 3->7
gene 1 4 6 0.0 0 1 0 1 # link 4->6
gene 1 5 7 0.0 0 1 0 1 # link 5->7
genomeend 1
```

The syntax is:

Trait (placeholder, always use this):
```
trait 1 0.1 0 0 0 0 0 0 0
```

Node:
```
node NodeID TraitNum (NodeType: 0 - neuron, 1 - sensor (can be used for input) (NodeLabel: 0 - hidden, 1 - input, 2 - output, 3 - bias)
```

Gene:
```
gene TraitNum InputNodeID OutputNodeID Weight IsRecurrent InnovationNumber MutationNumber Enabled
```

For InnovationNum just use incrementing numbers.
