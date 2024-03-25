# SimulIDE 

Electronic Circuit Simulator

**SimulIDE is a simple real time electronic circuit simulator**, intended for hobbyist or students to learn and experiment with analog and digital electronic circuits and microcontrollers.
It supports PIC, AVR, Arduino and other MCUs and MPUs.

**Simplicity, speed and ease of use** are the key features of this simulator.
You can create, simulate and interact with your circuits within minutes, just drag components from the list, drop into the circuit, connect them and push the “power button” to see how it works.

Simulation speed is one of the most relevant characteristics of this simulator.
It has been deeply optimized to achieve excellent speeds and low cpu usage.

SimulIDE also features a code Editor and Debugger for Arduino, GcBasic, PIC asm, AVR asm and others. It is possible to write, compile and do basic debugging with breakpoints, watch registers and global variables.


## Building SimulIDE:

Build dependencies:

 - Qt5 dev packages
 - Qt5Core
 - Qt5Gui
 - Qt5Xml
 - Qt5Widgets
 - Qt5Concurrent
 - Qt5svg dev
 - Qt5 Multimedia dev
 - Qt5 Serialport dev
 - Qt5 qmake

 
Once installed go to build_XX folder, then:

```
$ qmake
$ make
```

In folder build_XX/executables/SimulIDE_x.x.x you will find executable and all files needed to run SimulIDE.



## Running SimulIDE:

Run time dependencies:

 - Qt5Core
 - Qt5Gui
 - Qt5Xml
 - Qt5svg
 - Qt5Widgets
 - Qt5Concurrent
 - Qt5 Multimedia
 - Qt5 Multimedia Plugins
 - Qt5 Serialport


No need for installation, place SimulIDE folder wherever you want and run the executable.


