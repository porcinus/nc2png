# nc2png  
Generate preview and time estimations based on gcode file (milling), English and French localized.  
   
Based on a PHP script I made years ago to help me figure out time required to mill parts as well as toolpaths preview.  
  
Uses [libGD](https://libgd.github.io/), [libpng](http://www.libpng.org/) and [zlib](https://zlib.net/). Require [Ansicon](https://github.com/adoxa/ansicon) to be installed for Windows version.  
Require zlib1g-dev, libpng-dev, libgd-dev to be installed on Linux in order to compile.
  
  
#### Limitations (may be implemented in the future):  
- UTF support.  
- This code is not really optimized and a bit messy.  
- Doesn't support axis accelerations.  
  
  
#### Limitations (low chance of implementation):  
- Units : Doesn't take account of G20 (inch), will be considered as mm.  
- Work plane : Only support G17 (XY), G18-19 are ignored.  
- Tool compensation : Only support no compensation (G40), G41-42 are ignored.  
  
  
#### Features :  
- Work in absolute and relative mode (G90-91-90.1-91.1), note: Relative mode not fully tested.  
- Support linear as well as circular moves (G0-1-2-3).  
- Partial support of drilling operations (currently G81-82-83-80).  
- Output a "full" report containing detailled times and travel distances.  
- Multi-language support based on system language (currently only support EN/FR).  
- Can be compiled on Windows and Linux (may depend on distro).  
  
  
#### Preview features :  
- Partially compatible with Cutview codes (used to include visual tool width).  
- Different colors depending on the kind of move.  
- If containing Cutview tool definition, display "cut" toolpaths with variable color depending on cut depth.  
  
  
#### Preview :
![report](img/prev01.png)  
![preview](img/prev02.png)  
  
  
#### Usage :  
Extract files wherever you want than drag a .nc/.gcode or any file that contain Gcode.  
You can also set the program as default open.  
  
  
### Compile :  
Note before start:  
If you want to try this program and don't trust my release, well, you will have hard time...  
I will only provide a 32bits version because many hobbyist run CNC software that require use of the parallel port to control theres machine (not allowed on W64 without use of a specific driver).  
To avoid as much hypocrisy as possible, all binaries I compile on Linux have debug symbols removed (can easily save 25-50% size).  
Something to keep in mind here is that compiling on Windows allow easier debugging but shouldn't be done if you plan to have some portability.  
Depending on the way you compile this program, overhead will be high (Windows, excl program, tested in a virgin W10 VM):
- VScode, Cygwin64 on Windows: 25 dlls from Cygwin dir with a total size of 10.8Mb, crash w/o notice/dump on W10.
- VScode, MSYS2 (32bits) on Windows: 27 dlls from MSYS64 mingw32 'bin' dir with a total size of 11.3Mb.
- VScode, MSYS2 (64bits) on Windows: 27 dlls from MSYS64 mingw64 'bin' dir with a total size of 10.7Mb.
- Cross compile via Linux, require only 3 dlls for a total of < 1Mb incl program (XP->W10), please note that compiling directly compile on Windows allow for easy debugging.
  
#### Using Cygwin64 and VScode:  
VScode should detect correct path, but crash without notice or dump on W10.  
Required DLLs : cygbz2-1.dll, cygexpat-1.dll, cygfontconfig-1.dll, cygfreetype-6.dll, cyggcc_s-seh-1.dll, cyggd-3.dll, cyggomp-1.dll, cygiconv-2.dll, cygimagequant-0.dll, cygjbig-2.dll, cygjpeg-8.dll, cyglzma-5.dll, cygpng16-16.dll, cygstdc++-6.dll, cygtiff-6.dll, cygwebp-7.dll, cygwin1.dll, cygX11-6.dll, cygXau-6.dll, cygxcb-1.dll, cygXdmcp-6.dll, cygXpm-4.dll, cygz.dll, libgd-3.dll, libpng16-16.dll  
    
- From sources downloaded from Github:
  - Update `.vscode\tasks.json` to correct Cygwin paths.
  
- Compile the program in VScode.
  
#### Using MSYS2 and VScode:  
Required DLLs (32bits or 64bits) : imagequant.dll, libbrotlicommon.dll, libbrotlidec.dll, libbz2-1.dll, libdeflate.dll, libexpat-1.dll, libfontconfig-1.dll, libfreetype-6.dll, libgcc_s_dw2-1.dll, libgd.dll, libglib-2.0-0.dll, libgraphite2.dll, libharfbuzz-0.dll, libiconv-2.dll, libintl-8.dll, libjbig-0.dll, libjpeg-8.dll, liblzma-5.dll, libpcre-1.dll, libpng16-16.dll, libstdc++-6.dll, libtiff-5.dll, libwebp-7.dll, libwinpthread-1.dll, libXpm-noX4.dll, libzstd.dll, zlib1.dll  
  
Important notes:
In following lines, you will have to replace {MSYS2_PATH} by your full path to MSYS2 e.g C:\msys64 for default path.  

- Download lastest package from [MSYS2](https://www.msys2.org/)
  
- Run MSYS2 MinGW and install needed packages :
  - 32bits : `pacman -S --needed base-devel mingw-w64-i686-toolchain mingw-w64-i686-libgd`
  - 64bits : `pacman -S --needed base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-libgd`
  
- Update Windows environnement variable 'Path' by adding proper path :
  - 32bits : `{MSYS2_PATH}\mingw32\bin`
  - 64bits : `{MSYS2_PATH}\mingw64\bin`  
  
- From sources downloaded from Github:
  - Update `.vscode\tasks.json` to correct MSYS2 paths.
  
- Compile the program in VScode.
  
#### Cross-compile on Linux (Ubuntu Server):  
Everything done here using terminal.  
Require `git`, `tar`, `mingw-w64`, `mingw-w64-tools`, `unzip`, `dos2unix` packages to be installed.  
  
`cd $HOME`  
`mkdir work`  
`cd work`  
`mkdir libs`  
`mkdir build`  
`cd libs`  
`wget https://zlib.net/zlib1211.zip`  
`unzip zlib1211.zip`  
`wget https://sourceforge.net/projects/libpng/files/libpng16/1.6.37/lpng1637.zip`  
`unzip lpng1637.zip`  
`wget https://github.com/libgd/libgd/releases/download/gd-2.3.2/libgd-2.3.2.tar.gz`  
`tar -xf libgd-2.3.2.tar.gz`  
  
Now lets prepare compilation scripts, in order to work this will have to be done in a specific order : `zlib`, `libpng` then `libgd`.  
All these scripts will have to be created in `$HOME/work/libs`, don't miss to chmod +x each scripts once saved.  
  
Create 'zlib.sh':  
```#!/bin/bash
rm ../build/zlib1.dll
cd zlib-1.2.11
PREFIXDIR=$HOME/work/libs/zlib-1.2.11
make -f win32/Makefile.gcc clean
make -f win32/Makefile.gcc BINARY_PATH=$PREFIXDIR/bin INCLUDE_PATH=$PREFIXDIR/include LIBRARY_PATH=$PREFIXDIR/lib SHARED_MODE=1 STATIC_MODE=1 PREFIX=i686-w64-mingw32- install
cd ..
cp zlib-1.2.11/zlib1.dll ../build/zlib1.dll
```
  
Create 'libpng.sh':  
```#!/bin/bash
rm ../build/libpng16-16.dll
cd lpng1637
export CC='i686-w64-mingw32-gcc -static-libgcc'
export ZLIBLIB=$HOME/work/libs/zlib-1.2.11/lib
export ZLIBINC=$HOME/work/libs/zlib-1.2.11/include
export CFLAGS="-g -static-libgcc"
export CPPFLAGS="-I$ZLIBINC"
export LDFLAGS="-L$ZLIBLIB"
export LD_LIBRARY_PATH="$ZLIBLIB:$LD_LIBRARY_PATH"
dos2unix -f config.guess config.sub configure configure.ac missing scripts/*.* *.c *.h
make clean
./configure --host=i686-w64-mingw32 --enable-static=libgcc --enable-shared --enable-static --prefix=$PWD/tmp
dos2unix -f libtool Makefile depcomp
make
make install
dos2unix -f tmp/include/libpng16/*.h
cd ..
cp lpng1637/.libs/libpng16-16.dll ../build/libpng16-16.dll
i686-w64-mingw32-strip ../build/libpng16-16.dll
```
  
Create 'libgd.sh':  
```#!/bin/sh --
rm ../build/libgd-3.dll
cd libgd-2.3.2
export CC=i686-w64-mingw32-gcc
export CFLAGS=" -g "
export LIBZ_CFLAGS=" -g "
export LIBZ_LIBS=" -lz "
export LIBPNG_CFLAGS=" -g "
export LIBPNG_LIBS=" -lpng "
./configure --host=i686-w64-mingw32 --enable-shared --disable-static --prefix=$PWD/tmp --with-zlib=$HOME/work/libs/zlib-1.2.11 --with-png=$HOME/work/libs/lpng1637/tmp
make clean
make
make install
cd ..
cp libgd-2.3.2/src/.libs/libgd-3.dll ../build/libgd-3.dll
i686-w64-mingw32-strip ../build/libgd-3.dll
```
  
Lets compile libraries, get the program :  
`dos2unix -f zlib.sh libpng.sh libgd.sh`  
`chmod +x zlib.sh libpng.sh libgd.sh`  
`./zlib.sh`  
`./libpng.sh`  
`./libgd.sh`  
`cd $HOME/work`  
`git clone https://github.com/porcinus/nc2png`  
`cd nc2png`  

Create 'compile.sh':
```#!/bin/sh
export libpng_path="$HOME/work/libs/lpng1637/tmp"
export zlib_path="$HOME/work/libs/zlib-1.2.11"
export libgd_path="$HOME/work/libs/libgd-2.3.2/tmp"
rm $HOME/work/build/nc2png.exe
rm $HOME/work/nc2png/src/nc2png.res.o
/usr/bin/i686-w64-mingw32-windres -F pe-i386 -i $HOME/work/nc2png/src/nc2png.rc -o $HOME/work/nc2png/src/nc2png.res.o
i686-w64-mingw32-g++ -static-libgcc -static-libstdc++ -s src/lang.cpp src/win.cpp src/ansicon.cpp src/config.cpp src/gdcore.cpp src/ncparser.cpp src/nc2png.cpp src/nc2png.res.o -o $HOME/work/build/nc2png.exe -L"$libpng_path/lib" -I"$libpng_path/include" -L"$zlib_path/lib" -I"$zlib_path/include" -L"$libgd_path/lib" -I"$libgd_path/include" -lgd -lpsapi -lm -lz -std=c++17
i686-w64-mingw32-strip $HOME/work/build/nc2png.exe
chmod 755 $HOME/work/build/nc2png.exe
```
  
Lets compile the program :  
`dos2unix -f compile.sh`  
`chmod +x compile.sh`  
`./compile.sh`  
  
All files you need should be found into `%HOME/work/build/` directory.  
  
#### Compile for Linux (Ubuntu Server):  
Assuming that you already have g++ packages installed, following packages will be required as well : `zlib1g-dev`, `libpng-dev`, `libgd-dev`.  
Clone the git wherever you want then cd into src directory.  
`g++ -s lang.cpp ansicon.cpp config.cpp gdcore.cpp ncparser.cpp nc2png.cpp -o nc2png -lgd -lm -lz -std=c++17`  
  
