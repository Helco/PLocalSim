# PLocalSim

## Overview

This project seeks to give developers a authentic build environment, which can be used to write, test and debug Pebble watchfaces/apps across the big operating systems Linux, Microsoft Windows and Apple Mac OS X. Instead of the emulator approach (which is currently done by the official developers of Pebble) this is mainly an implementation of the Pebble SDK.
> The last big update for this project changed a lot of the old structures to be easier to handle.  In the events of this update, I had to cut the support for Mac OS X for a moment. But stay tuned, it will return in one of the next updates.

<sup>Due to the new naming guidelines from Pebble Technology Inc. this project was renamed to PLocalSim</sup>

## Compatibility

The most important feature for this simulator is compability. I tested this simulator with the official examples ([here are the results](#ExampleCompatibility)) You should be able to leave your source code like it is for your watch, but there are a few problems:

*  __Non-implemented features__. Very simple. I will try to implement all features left as soon and as authentic as possible. Understandable there are some features which are more difficult to implement than others, for example the watch <-> phone connection. These features may come very late or worse. For a list of non-implemented features look [here](#NotImplemented)

*  __Bugs__. This simulator has lots of code and there are probably lots of little bugs crawling inside of it. I can not find all bugs but with your help the search will be much easier.

*  __Documentation__. Even though there is much more documentation for the official SDK than I expected, I can not make sure that all functions behave like their original ones. If you find any differences, please tell them to me.

*  __Fonts__. The fonts that are most common on the pebble are Bitham (renamed from Gotham) and Gothic. I presume that they are commercial fonts which could be extracted from the firmware, but this would be illegal. In the case that you have these fonts (as TrueTypeFont) you can replace them, as I filled the missing fonts up with Droid Serif and Roboto which have a free license (you can find the fonts and their license text in the directory `simdata/common/systemFonts`). The problem with these fonts are, that they are wider than the original ones, so some watchapps may have graphical bugs. In the most sample watches, where this was a problem, it could be solved with extending the layer frame width and reducing the margin.

## Non-implemented features<a name="NotImplemented"></a>
Please consider that this list may not complete as new features are added. This list was created with the Pebble SDK 2.8.1
 
*  __MenuLayer__			
*  __SimpleMenuLayer__
*  __NumberWindow__
*  __Accelerosensor__ (accel_tap_* is supported)

## Watch Apps from SDK <a name="ExampleCompatibility"></a>
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

### No working

These do not work:

*  ninety_one_dub - (Probably invalid? day_name_layer is being used without initialising)


Pretty good result, isn't it :)

## Installation
Since the first release of this project, I restructured the whole installation/building process to be easier and equal across the operating systems. Please read the whole section before trying anything of this out.

#### Common
1. Download this whole repository and the Pebble SDK archive 
2. Copy the headers from the Pebble SDK archive (PebbleSDK-X.X.X/Pebble/include/*.h) into the `include` folder of this repository. <sup>Due to copyright reasons I may not distribute these files.</sup>
3. Navigate in a bash shell to the extracted main folder
4. Run `install.sh` (optional you can specify a path where to install the sdk) 

#### Windows
On windows you can obviously not build projects for the actual pebble watch, but you can use the simulator without the official Pebble SDK. You have to specify a path to `install.sh` and create projects with `create.project.sh <path_to_project>`.
Besides that you need to provide the SDK with a recent version of MinGW, MSYS and SDL2. Of course you can use pre-installed versions of that, but to make everything work you would have to modify these files:

* openShell.bat
* envvars.sh
* sdkdata/windows/openShell.bat
* sdkdata/windows/envvars.sh

As alternative I have prepared two archives you can extract right in the main folder of this repository to have MinGW, MSYS and SDL2 pre-installed and pre-configured. These files will be copied into the installed SDK-folder, so this should work right out-of-the-box when you use this method. Here are the links:
> TODO: Actually entering the links would be very helpful, don't you think? - Helco

Of course these preparations have to be made before you even try to run `openShell.bat` which by the way just opens a bash shell in the current directory.

#### Linux
Before you run `install.sh` you should make sure that you have the right packages installed. <sup>32/64 Bit problems like earlier should not happen anymore.</sup>
On Ubuntu based distributions you can do this check by running this command in a shell:
`sudo apt-get install libsdl1.2-dev libsdl-image1.2-dev libsdl-ttf2.0-dev`
If you don't specify a path to `install.sh`, the script tries to find your pebble sdk path (by searching the pebble executable) and installs itself into the very path. If you don't specify a path, this project installs itself like the windows SDK: You can create projects with `create.project.sh <path_to_project>`. To be able to use these projects with the official Pebble SDK you would have to write/copy at least the `wscript` from another project.
> TODO: Helco don't leave this as it is before release!!!
> Make sure that we copy the headers and provide a create.project.sh, just do it

#### Mac OS X
Unfortunately I had to drop the support for Mac OS X for the moment, but one of the next updates should reactivate this.

## Usage
The usage for building watchfaces/apps are very simple. Just run `build.local.sh` in any project folder to build the project as local simulator and then run `run.local.sh` to start the simulator.
If you want to debug with GDB you can run these scripts with the parameter `--debug`.
If you want to help me with the development of the simulator you can use the `build.library.sh` and the `build.resCompiler.sh` script to build the various parts of the simulator.

## Simulator keyboard usage

The simulator has several key commands you can use:

| Key          | Action                                              |
| :----------- | :-------------------------------------------------- |
| [Arrow up]   | Pebble button: Up                                   |
| [Arrow down] | Pebble button: Down                                 |
| [Enter]      | Pebble button: Select                               |
| [Backspace]  | Pebble button: Back                                 |
| [+]          | Add 10% charge to the battery                       |
| [-]          | Drain 10% charge from the battery                   |
| [x][y][z]    | Accelerometer tap on the +X/+Y/+Z axis              |
| [X][Y][Z]    | Accelerometer tap on the -X/-Y/-Z axis              |
| [F1]         | Changes the color of the drawn pebble               |
| [F2]         | Toggles the lens effect                             |
| [F3]         | Switches between 24H and 12H mode                   |
| [F4]         | Toggles bluetooth on / off                          |
| [F5]         | Toggle charger plugged / not plugged                |
| [F12]        | Saves a screenshot in <simulator>\screenshots\      |

## Disclaimer and Licenses

*  The fonts are copyright by Google Inc. under Apache License, Version 2.0
*  The [pebble pictures](https://github.com/pebble/PebbleUI) are (now) by Pebble Technology Inc. under [CC BY-SA 4.0](http://creativecommons.org/licenses/by-sa/4.0/deed.en_US)
*  The [used libraries](#UsedLibraries) are copyright by their copyright holder (not me)
*  The python based resource compiler was placed in the public domain by Shirk
*  `additionalSource/strftime.c` stands under Public Domain (is part of MinGW I suppose)
* PLocalSim (this repository minus all otherwise stated parts) is licensed by Helco (me) under [GNU GPL v3](http://www.gnu.org/copyleft/gpl.html)
* PLocalSim is an independent software and has not been authorized, sponsored, or otherwise approved by Pebble Technology Corp.

## Credits

### Many many thanks to:

*  [ae-code](https://github.com/ae-code) - for the Javascript, Dictionary, AppSync and AppMesage API
*  [David Konsumer](https://github.com/konsumer) - for a new README (with beatiful Markdown)
*  [robhh](https://github.com/robhh) - for the AppTimer implementation
*  [abl](https://github.com/abl) - for empebble (which was original the basecode of this simulator, now all code is rewritten)
*  [AmandaCameron](https://github.com/AmandaCameron) - for the metadata support implementation (in PebbleSDK 1.2)
*  [epatel](https://github.com/epatel) - for the earlier mac support
*  [Shirk](https://github.com/Shirk) - for the python based resCompiler for unix systems
*  [Pebble Technology Inc.](https://developer.getpebble.com/) - for the amazing watch, the SDK and their long shipping procedure, which was the motivation to build this :)
* Everyone who provided me with bug reports, ideas and feedback

### Used libraries/code <a name="UsedLibraries"></a>

*  [node-XMLHttpRequest](https://github.com/driverdan/node-XMLHttpRequest/blob/master/lib/XMLHttpRequest.js)
*  [node-localStorage](https://github.com/ae-code/node-localStorage)
*  [SDL2](www.libsdl.org)
*  [SDL2_ttf](http://www.libsdl.org/projects/SDL_ttf/)
*  [SDL2_image](http://www.libsdl.org/projects/SDL_image/)
*  [SDL_gfx](http://cms.ferzkopp.net/index.php/software/13-sdl-gfx)
*  [JSMN](https://bitbucket.org/zserge/jsmn)
