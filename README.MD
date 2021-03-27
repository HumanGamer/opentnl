# Torque Network Library
## Readme

##### (c) GarageGames 2004

## Introduction
Developing computational simulations is a complex, interdisciplinary undertaking. To take a simulation of any sort and run it in a distributed environment introduces a whole new world of problems to overcome. The Torque Network Library provides a robust framework for message  passing and efficient state replication, built on a UDP-based protocol.

## Features

* Portable
	* Platforms
		* Mac OSX
		* Linux
		* Win32
	* Compilers
		* Xcode
		* GCC
		* Visual C++ 6/7
* Powerful RPC Support
	* Lightweight and efficient - not intended to be a replacement for CORBA.
	* Extremely easy to use - two simple macro definitions and you're done.
	* No complex wrapper classes to work with - write and call your functions as you normally would, and TNL will take care of the rest.
* Robust UDP-based notification protocol
	* Supports fixed or adaptive rate data transfer.
	* Arbitrary sized packet window.
	* Robust connection handshaking.
	* Handles packet loss/out of order delivery.
* Networked Objects
	* Instantiation of objects by name or ID.
	* IDs are assigned to NetGroups, so that only objects relevant to a connection can be references.
* Event Support
	* Unguaranteed, guaranteed unordered, or guaranteed ordered event delivery.
	* Notification to the event when it has been received or dropped by a client.
* Ghost Support
	* Most-recent state information is transmitted to clients.
	* Scoping of objects to only those clients for which they are relevant - this helps prevent cheating or data sniffing and makes dramatically more effective use of available bandwidth.
	* Prioritization of state updates by arbitrary criteria, such as relative velocity, team, ownership, etc.
* Extensible Protocol Architecture
	* Easy to extend the protocol, for instance, to transmit player move information with every packet, or add voice support.
	* See Torque for examples of additional capabilities that could be added to the protocol.

## Compiling TNL

### OSX/Xcode

Under the ```osx``` subdirectory there is an XCode project. It is set up to build TNL and the TNLTest app.

### Linux/GCC

From the top level directory, type

```make```

If you have OpenGL and wxWindows installed, and OpenGL support enabled in wxWindows, you can build the wxgui test client/server program with

```cd test && make wxgui```

The wxgui is not built by default.

### Windows/VC6/VC7

There are VC6 and VC7 project workspaces in the root directory of the checkout. Individual project files are in their respective directories.

## Documentation and Online Resources

The TNL comes with extensive Doxygen documentation. You can access this documentation online at [Some URL](), or generate it by downloading Doxygen from [www.doxygen.org](http://www.doxygen.org/) and executing the following command line (assuming you installed Doxygen in the standard place) from the root of your TNL checkout...

* Under Windows:
	```"c:\Program Files\doxygen\bin\doxygen.exe" docs\doxygen.html.cfg```
* Under Linux or OSX:
	```doxygen docs/doxygen.html.cfg```

This will produce a set of Doxygen docs in the docs\html subdirectory of your TNL checkout. Please note that these docs may ONLY be distributed to other TNL licensees.