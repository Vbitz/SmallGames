# Bug List

- Fix OpenGL Contexts on Linux
- Fix remaining rendering bugs on older scripts due to the new batching
- Fix V8 Classes (Probaly something they broke in an update)
	- It's still a feature in beta, maybe they deactivated it or changed the command line to reactivate it

# Task List

These are not necessarily in order that they will be completed.
Most of these come from my internal task list so don't take this as final.

- Finish new Script Binding systems
	- Finish adding new functions to api.js
	- Add support for JS Classes
	- Add support for variable arguments
- Native Window Contexts ??
	- Add WindowOSX
	- Add WindowLinux
	- Add WindowWin
	- I'm not really sure this is needed anymore with the ease of using GLFW
		- I'd only really consider changing it if GLFw holds the project back
- Autoset configDir based on filename
	- You should be able to override this in the configs
- Change logging to a rotating buffer
	- This is one of the main memory issues right now
- Add Missing Textures
- Report to Windows
	- **DONE** Stub Out Platform_win.cpp
	- **DONE** Fix linking issues and install library's
	- **DONE** Implement Platform_win.cpp
	- Fix Runtime Bugs
		- Finish Memory Mapped File support
			- The plan to handle the lack of expanding memory mapped files in windows is to reserve virtual memory space and map the current file size exactly to the region
		- Fix switch to fullscreen (Might use NSight)
	- **DONE** Profile performance
		- I expected more of a performance gap between a GTX970 vs Intel HD 5000, there's next to none right now
		- NSight worked well for tracking down the performance difference, it seems the GPU is idling most of the time and it's nothing to worry about.
		- **DONE** Fix performance fluctuations on windows
			- This is a combination of less state changes and defering script updating frame by frame
	- **DONE** Release a testing version
	- **DONE** Commit back to upstream
	- Add release task support for windows
		- Probaly just generate the same exact thing I do manuly right now
		- It would be nice to trim the resources dir though
	- Add CI Support with http://www.appveyor.com/
		- I've done inital tests but the depenandancys end up being too slow to build
		- the best solution seems to be to make a 40mb archive of the depends prebuilt and use that
		- Useful to make the windows developer ux better as well
	- **DONE** Add console color support to windows
	- Add Release config for Win32 VS Project
		- No Debug Info
		- Maybe add PROFILER automaticly to the Debug config
	- Add 64-bit Windows Option
- Port to Linux
	- Fix Windows/OpenGL on linux
	- Add linux release uploading to Travis
- Add new Scripting Features
	- Add require system
		- This will likely be based off require js or node's require system
		- For larger projects this is likaly one of the higher priority topics due to how hard the current system makes it to manage tons of files
	- Implement new functions in javascriptAPI.txt
		- Update and extend javascriptAPI.txt
		- I think the design should focus even more around a object orintated design although I need to consider what effect
			that has on support for other scripting engines
	- Implement support for Packages in Javascript
		- There's a little right now but it should be full support
- Extend Packages
	- Patching support
	- Hash/Sign/Verify support
	- Encryption support
	- Allow overwrite of existing sectors
- Audio Support
	- Add OpenAL Depend (maybe another library, something to really look into)
	- Write C++ Classes
	- Write JS API (Maybe raw sound or maybe something more like web audio
- Support C based add-ons
	- Engine2D.h common API header file with wrapper code
		- Wrappers for important classes
			- Application
			- Window
			- Events
			- Draw2D
			- RenderDriver
		- In the future I could add C as a possible target for the script binding system
- Entity System
	- Abstract out messaging to work with entity's
	- Implement Serialization for the level format
- UI System
	- UI specific event systems
	- UI rendering framework
	- UI layout framework
	- UI Elements
		+ Button
		+ Listbox
		+ Textbox
		+ Checkbox
		+ Layout
		+ etc...
	- Loading UI from a source format
- Crypo/Security Features
	- Add OpenSSL
	- Add signed scripts/executable support
	- Add package encryption support
		- Add PackageEncryptionProvider
- Add Other scripting engine support
	- Remove all instances of v8::Isolate:GetCurrent
		- I heard that v8 will be removing this or deprecating it soon
	- Remove all instances of v8 outside of ScriptingManager
		- Most of these will be removed with the shift away from the old binding models
	- Add support for other Engines
		- LuaJit
			- Likely to become a included by default platform
		- Python
		- Java ??
			- Useful for android maybe
		- C# ??
			- With Microsoft's recent changes this could be done with their API
- Networking
	- The networking system will be tier based like the current rendering system
		- Enet
			- Get a feel for the libray
			- Write a small test in the engine with a very simple script binding
			- Get a feel for the performance and overhead
		- Events::NetworkTarget / EntityManager::NetworkSync ??
	- I'll ether roll my own sockets or use enet
- Multi-Threaded Rendering
	- Make rendering code thread safe/thread local
	- Multi-Thread JavaScript/Other scripting
- 3D Support
	- Lighting
		- Fully deferred rendering
		- Physically based rendering
	- Draw3D
		- I need to consder if this is the best aprouch to 3D or is a more advanded shape + entity system like Three.JS more effective
