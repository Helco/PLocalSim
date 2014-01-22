# PebbleLocalSimulator

### Version: 0.2 BETA

## Overview

This project is an extension to the PebbleSDK2.0 Beta2 which allows you to build and run your pebble applications on your computer.

## Usage

The simulator has several key commands you can use:

```
[Arrow up]	 Pebble button: Up
[Arrow down] Pebble button: Down
[Enter]		 Pebble button: Select
[Backspace]	 Pebble button: Back
[+]		     Add 10% charge to the battery
[-]		     Drain 10% charge from the battery
[x][y][z]	 Simulate an accelerometer tap on the X/Y/Z axis with +1 direction
[X][Y][Z]	 Simulate an accelerometer tap on the X/Y/Z axis with -1 direction
[F1]		 Changes the color of the drawn pebble
[F2]		 Toggles the lens effect 
[F3]		 Switches between 24H and 12H mode
[F4]		 Toggles bluetooth on / off
[F5]		 Toggle charger plugged / not plugged
[F12]		 Saves a screenshot in <simulator>\simdata\screenshots\
```

## Non-implemented features

There are some things that are not implemented yet:
 
*  __MenuLayer__			
*  __SimpleMenuLayer__
*  __NumberWindow__
*  __Accelerosensor__ (accel_tap_* is supported)

Except from these,  all other features should be implemented

## Compatibility

The most important feature for this simulator is compability. You should be able to leave your source code like it is for your watch, but there are a few problems:

*  __Non-implemented features__. Very simple. I try to implement all features left as soon as possible. At the moment this simulator is my only possibility to actual code for pebble, so it is in my own interest to work as quick and as best as I can.

*  __Bugs__. This simulator has lots of code, lots of unverified or not enough verified code and there are probably lots of little bugs crawling inside of it. This is an important reason why I release it now. I can not find all bugs but with your help the search will be much easier.

*  __Documentation__. Even though there is much more than I expected, I can not make sure that all functions behave like their original ones. If you find any differences, please tell them to me.

*  __Fonts__. The fonts that are most common on the pebble are Bitham (renamed from Gotham) and Gothic. I presume that they are commercial fonts which could be extracted from the firmware, but I think this would be illegal. In the case that you have these fonts (as TrueTypeFont) you can replace them, as I filled the missing fonts up with Droid Serif and Roboto which have a free license (you can find the fonts and their license text in the directory `local/simdata/systemFonts`). The problem with these fonts are, that they are wider than the original ones, so some applications have graphical bugs because of this. In the most sample watches, where this was a problem, it could be solved with extending the layer frame with and reducing the margin.


## Watch Apps from SDK

I tested the sample watches from the SDK to detect these problems:

### Totally work

These work, without issues:

*  big_time
*  drop_zone
*  fuzzy_time
*  just_a_bit
*  rumbletime
*  segment_six
*  simple_analog
*  simplicity
*  tic_tock_toe

### Minor font issues

These work, with minor font/layout issues:

*  classio
*  classio-battery-connection
*  onthebutton

### No worky

These do not work:

*  ninety_one_dub - (Probably invalid? day_name_layer is being used without initialising)


Pretty good result, isn't it :)

## Compatibility between different operating systems


__I want YOU!__ to find:

*  bugs
*  suggestions
*  feedback
*  questions

and [tell them to me](https://github.com/Helco/PebbleLocalSim/issues). I will try to answer them all.

## Installation

### Linux

1. Copy the content of the directory `./build/linux/build/` into your pebble sdk directory (e.g. `/home/helco/pebble/PebbleSDK-2.0/`) It will not overwrite any original files. 
2. It may be that you have to install some SDL libraries if you haven't done this before. On ubuntu this requires one simple command: `sudo apt-get install libsdl1.2-dev libsdl-image1.2-dev libsdl-ttf2.0-dev`

To build your app, switch (in the terminal) to your project's main directory and run `build.local.sh`

This will build your application, compile your resources and put all files necessary to run
your application in the directory `<project>/build/local/`. You can run it from there or simply type: `run.local.sh` which will run it for you.

#### Debugging

If you see the need you can add the flag `--debug` to build.local.sh or `run.local.sh` to build your project with debugging symbols and run it in gdb.

#### Build the project on your own

To build the project on your own, there are two codeblocks projects in the directory `build/linux/codeblocks`


### Mac

Install SDL, SDL_ttf, SDL_image with homebrew, use `--universal` to get 32 bit support.
`brew install SDL SDL_ttf SDL_image --universal`

In build/mac there is a pre-built `sim.zip` package ready to be used.

Unzip and place the sim directory in your pebble project. 

Open Terminal.app and cd into it

`make`

It should compile all `.c` files in src.

Hopefully you will get a PebbleSim binary you can run from the terminal with

`./PebbleSim`

Good luck!

#### Debugging

Follow the steps for normal use but build and run with the following commands:

`make debug`

It should compile all `.c` files in src with debugging enabled.

Run the debug simulator in gdb:

`gdb ./PebbleSim.debug`

#### Build the project on your own

Open Terminal.app and cd into `build/mac`

`make`

Hopefully you now have a sim directory ready to be used as described above.


### Windows

You can use the the directory `.\build\windows\` as a windows sdk but if you like to have it somewhere else (probably easier to access) you can follow these instructions:

1. Switch to your chosen directory and type `<original_sdk>/build/windows/install.bat`
2. This projects requires MinGW. You have to set the bin path of it in the `<sdk>/envvars.bat` file or you can leave it at default and extract the archives listed below into the folder `<sdk>\MinGW\`

Note: If you do these changes before you use install.bat your installed sdk will have your
changes too.

#### MinGW Archives:

*  [gcc-core](http://prdownloads.sf.net/mingw/gcc-core-3.4.5-20060117-1.tar.gz)
*  [gcc-g++](http://prdownloads.sf.net/mingw/gcc-g++-3.4.5-20060117-1.tar.gz)
*  [mingw-runtime](http://prdownloads.sf.net/mingw/mingw-runtime-3.9.tar.gz)
*  [mingw-utils](http://prdownloads.sf.net/mingw/mingw-utils-0.3.tar.gz)
*  [w32api](http://prdownloads.sf.net/mingw/w32api-3.6.tar.gz)
*  [binutils](http://prdownloads.sf.net/mingw/binutils-2.17.50-20060824-1.tar.gz)
*  [mingw32-make](http://prdownloads.sf.net/mingw/mingw32-make-3.81-1.tar.gz)

Unlike on other operating systems, the pebble team has no sdk for windows yet, that is why you have to create a project with a tool provided by this sdk. To create a project switch  to the directory you want your projects folder in and run the command: `create.project.bat <project_name>`

This will create a sample project for you to start coding. To build your project run: `.\tools\build.local.bat` from your projects main folder. To run it you can switch to the directory `<project>\build\local\` or you can run: `.\tools\run.local.bat` also from your projects main folder.

#### Build the project on your own

To build an installation of this pebble simulator sdk switch to the directory `.\build\windows`.

There you have two commands:
*  `build.library.bat` to build the library used to create the simulator
*  `build.resCompiler.bat` to build the resource compiler

Note that you need MinGW (see [Installation]) to build this project

## License

*  The fonts are copyright by Google Inc. under Apache License, Version 2.0
*  The pebble pictures are copyright by Chaotic
*  The used libraries (see __Credits__) are copyright by their copyright holder (not me)
*  The actual simulator and the custom resource compiler (C version) are licensed by Helco (me) under GNU GPL v3
*  The python based resource compiler was placed in the public domain by Shirk
*  `additionalSource/strftime.c` stands under Public Domain (is part of MinGW I suppose)

## Credits

### Many many thanks to:

*  [ae-code](https://github.com/ae-code) - for the Javascript, Dictionary, AppSync and AppMesage API
*  [David Konsumer](https://github.com/konsumer) - for the new README
*  [robhh](https://github.com/robhh) - for their AppTimer implementation
*  [Otto Greenslade (Chaotic)](http://dribbble.com/chaotic) - for their pebble pictures
*  [abl](https://github.com/abl) - for their empebble (which was original the basecode of this simulator, now nearly all code is rewritten)
*  [AmandaCameron](https://github.com/AmandaCameron) - for their metadata support implementation (in PebbleSDK 1.2)
*  [epatel](https://github.com/epatel) - for their mac support
*  [Shirk](https://github.com/Shirk) - for creating the python based resCompiler for unix systems
*  [The Pebble Team](https://developer.getpebble.com/) - for the amazing watch, the SDK and their long shipping procedure, which was the motivation to build this

### Libraries/Code that are used:

*  [node-XMLHttpRequest](https://github.com/driverdan/node-XMLHttpRequest/blob/master/lib/XMLHttpRequest.js)
*  [node-localStorage](https://github.com/ae-code/node-localStorage)
*  [SDL 1.2](www.libsdl.org)
*  [SDL_ttf](http://www.libsdl.org/projects/SDL_ttf/)
*  [SDL_image](http://www.libsdl.org/projects/SDL_image/)
*  [SDL_gfx](http://www.ferzkopp.net/joomla/content/view/19/14/)
*  [JSMN](https://bitbucket.org/zserge/jsmn)
