# Bouncy Balls

### Building

If you have the Emscripten SDK, you can build by running

```bash
# on Windows
./build.ps1

# on Linux
./build.sh
```

If you don't have the Emscripten SDK, you need to install it.

From [the official docs](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html):
```bash
# run these commands in a shell
# in the folder where you keep your code / git clones
git clone https://github.com/juj/emsdk.git
cd emsdk
./emsdk update
# this will take some time
./emsdk install latest
./emsdk activate latest
# close the shell and reopen in the project folder
# you should now be able to build
# and run the emcc and emrun commands
```