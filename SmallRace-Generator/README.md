# Smalltalk

## Build

With LLVM 12.0.1, we use the following commands to build SmallRace-Generator:

```
mkdir build && cd build
cmake ..
make -j4
```

## Run

Run **st-racedetect** under *./build/bin/*. 

```
./st-racedetect ../../examples/ --dump-ir
```
The LLVM IR output will be generated under the current workspace at ./build/bin, naming **t.ll**.

