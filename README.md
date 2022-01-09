![](doc/images/ggpo_header.png)

&nbsp; _[![Appveyor build status](https://img.shields.io/appveyor/ci/pond3r/ggpo/master.svg?logo=appveyor)](https://ci.appveyor.com/project/pond3r/ggpo/branch/master)_

(日本語ドキュメントは[こちら](README.ja.md))

## What's GGPO-vNext?
After some discussion in the ggpo discord channel, we came to the conclusion that the ggpo project is not complete and that people have their own build of ggpo to add changes/improvements. One of the reasons might be the fact that pull-requests in the original ggpo project are not beeing reviewed & merged anymore. So I will manage this version & will look into adding more contributers to manage. I have listed made a list of changes/goals to finish for this version of ggpo.
* :white_check_mark: Abstracting the connection layer
* Removing the need for global variables.
* Server based rollback game ability: something similair to brawlhalla. (this will be optional)
* A system where the clients sync their gamestate every X frame by comparing checksums: this is for the devs that can't get determinism working, it makes cheating easy to do tough.(Luckily *EOS has a built in anti-cheat for free, an example for this will be provided) (this will be optional).
* Full *EOS example.
* A server based input relay: Currently ggpo sends all the inputs from the last received ack from the opponent. Ideally I would send only the last input(or last 2 inputs, the amount could be made to be configurable) to the server and if an input drops the server will tell the client to rollback and fix their own input. the server will send the input to the opponent, or to a server close by to the opponent that can relay it to the opponent (aws accelerators could be awesome for this) (this will be optional).
* Another callback called tick. Currently the dev executes their own code and executes the ggpo_advance_frame. After rollback the advance_frame callback get executed in which they have to execute their game logic. Ideally I would let the dev put their game logic inside the tick callback and call it inside the new ggpo when advance_frame is executed. This could be coupled with a accumulator logic to prevent frame drops in code. + This way the rift fixing can also be already implemented inside ggpo instead of letting the dev handle it. (as we have seen, even AAA devs mess this up. Maybe it's not a good idea to let devs fix rift. IMO the framework should fix it. I will do the rift fixing by time scale. The player ahead will execute at 0.75 speed while player behind will execute at 1.25 speed)
* Switch the project to premake from cmake? I have heard from some devs that they gave up on ggpo bcuz they couldn't get cmake to work.
* Move all the #define parameters like for ex. MAX_PREDICTION_FRAME to a config struct: This would be nice for devs that use other languages using wrappers; they wouldn't have to recompile after each change.
* Make another ggpo_start method for offline play with delayed inputs & ability to rollback x amount of frames (this can be handy for practice mode and debugging)
* Add replay functionality (save inputs, and reload them and play them)
* Create a json file containing all the ggponet.h functions and args. This can be used to automate creating wrappers to other languages.
* CREATE UNIT TEST! Currently there is no automated tests for ggpo.
* Abstract all platform dependent methods and make the injectable. This way ggpo can have support for all platforms + if you use an engine, you can use the engine's cross platform implementation for these methods. (ex. for GetCurrentTimeMS you could use unreal engine's own time methods instead of the windows one)

EOS = Epic Online Services. A cross platform service that provides matchmaking, nat traversal, anti cheat, voip, etc.
## What's GGPO?

Traditional techniques account for network transmission time by adding delay to a players input, resulting in a sluggish, laggy game-feel.  Rollback networking uses input prediction and speculative execution to send player inputs to the game immediately, providing the illusion of a zero-latency network.  Using rollback, the same timings, reactions, visual and audio queues, and muscle memory your players build up playing offline will translate directly online.  The GGPO networking SDK is designed to make incorporating rollback networking into new and existing games as easy as possible.  

For more information about the history of GGPO, check out http://ggpo.net/

This repository contains the code, documentation, and sample applications for the SDK.

## Building

Building GGPO is currently only available on Windows, however efforts are being made to port it to other platforms.

### Windows

Windows builds requires both [Visual Studio 2019](https://visualstudio.microsoft.com/downloads/) and [CMake](https://cmake.org/download/).  Make sure you've installed both before starting.  Make sure to add CMake to your path when installing.

- Run the `build_windows.cmd` in the root directory of the SDK to generate the Visual Studio 2019 solution files.   
- Open `build/GGPO.sln` solution for Visual Studio 2019 to compile.

You can also run the `cmake-gui` tool if you prefer. 

## Sample Application

The Vector War application in the source directory contains a simple application which uses GGPO to synchronize the two clients.  The command line arguments are:

```
vectorwar.exe  <localport>  <num players> ('local' | <remote ip>:<remote port>) for each player
```

See the .cmd files in the bin directory for examples on how to start 2, 3, and 4 player games.

## Licensing

GGPO is available under The MIT License. This means GGPO is free for commercial and non-commercial use. Attribution is not required, but appreciated. 
