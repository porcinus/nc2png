# nc2png  
Generate preview and time estimations based on gcode file (milling), English and French localized.  
  
For now, this repo is a stud...  
  
Based on a PHP script I made years ago to help me figure out time required to mill parts as well as toolpaths preview.  
  
Uses [libGD](https://libgd.github.io/), [libpng](http://www.libpng.org/) and [zlib](https://zlib.net/). Require [Ansicon](https://github.com/adoxa/ansicon) to be installed for Windows version.  
Require zlib1g-dev, libpng-dev, libgd-dev to be installed on Linux in order to compile.
  
  
#### Limitations (may be implemented in the future):  
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
- If compiling from VScode, rename 'vscode' to '.vscode'.  
- Instructions are be provided "as is".  
- If you want to try this program and don't trust my release, well, you will have hard time...  
- To avoid as much hypocrisy as possible, all binaries I compile on Linux have debug symbols removed (can easily save 25-50% size).  
- Yes, I checked every single files one by one on a vigin Win10 VM.  
- Compile cross compile process on Linux (Ubuntu Server) will come later.  
- Depending on the way you compile this program, overhead will be high (Windows, excl program):

Some numbers because why not...
- VScode, Cygwin64 on Windows: 25 dlls from Cygwin dir with a total size of 10.8Mb, crash w/o notice/dump on W10.
- VScode, MSYS2 (32bits) on Windows: 27 dlls from MSYS64 mingw32 'bin' dir with a total size of 11.3Mb.
- VScode, MSYS2 (64bits) on Windows: 27 dlls from MSYS64 mingw64 'bin' dir with a total size of 10.7Mb.
- Manual library compile, require only 3 dlls for a total of < 1Mb incl program (XP->W10), program cross compile on Linux will be added later, please note that compiling directly compile on Windows allow for easy debugging.
  
#### Using Cygwin64 :  
VScode should detect correct path, but crash without notice or dump on W10.  
- Required DLLs (25 files for 10.8Mb):
  - cygbz2-1.dll
  - cygexpat-1.dll
  - cygfontconfig-1.dll
  - cygfreetype-6.dll
  - cyggcc_s-seh-1.dll
  - cyggd-3.dll
  - cyggomp-1.dll
  - cygiconv-2.dll
  - cygimagequant-0.dll
  - cygjbig-2.dll
  - cygjpeg-8.dll
  - cyglzma-5.dll
  - cygpng16-16.dll
  - cygstdc++-6.dll
  - cygtiff-6.dll
  - cygwebp-7.dll
  - cygwin1.dll
  - cygX11-6.dll
  - cygXau-6.dll
  - cygxcb-1.dll
  - cygXdmcp-6.dll
  - cygXpm-4.dll
  - cygz.dll
  - libgd-3.dll
  - libpng16-16.dll
  
#### Using MSYS2 :  
- Required DLLs (32bits) (27 files for 11.3Mb):  
  - imagequant.dll
  - libbrotlicommon.dll
  - libbrotlidec.dll
  - libbz2-1.dll
  - libdeflate.dll
  - libexpat-1.dll
  - libfontconfig-1.dll
  - libfreetype-6.dll
  - libgcc_s_dw2-1.dll
  - libgd.dll
  - libglib-2.0-0.dll
  - libgraphite2.dll
  - libharfbuzz-0.dll
  - libiconv-2.dll
  - libintl-8.dll
  - libjbig-0.dll
  - libjpeg-8.dll
  - liblzma-5.dll
  - libpcre-1.dll
  - libpng16-16.dll
  - libstdc++-6.dll
  - libtiff-5.dll
  - libwebp-7.dll
  - libwinpthread-1.dll
  - libXpm-noX4.dll
  - libzstd.dll
  - zlib1.dll
  
- Required DLLs (64bits) (27 files for 10.7Mb):  
  - imagequant.dll
  - libbrotlicommon.dll
  - libbrotlidec.dll
  - libbz2-1.dll
  - libdeflate.dll
  - libexpat-1.dll
  - libfontconfig-1.dll
  - libfreetype-6.dll
  - libgcc_s_seh-1.dll
  - libgd.dll
  - libglib-2.0-0.dll
  - libgraphite2.dll
  - libharfbuzz-0.dll
  - libiconv-2.dll
  - libintl-8.dll
  - libjbig-0.dll
  - libjpeg-8.dll
  - liblzma-5.dll
  - libpcre-1.dll
  - libpng16-16.dll
  - libstdc++-6.dll
  - libtiff-5.dll
  - libwebp-7.dll
  - libwinpthread-1.dll
  - libXpm-noX4.dll
  - libzstd.dll
  - zlib1.dll
  
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
  
#### Using Cross-compile on Linux (Ubuntu Server):
TODO...libGD,libpng,zlib...
