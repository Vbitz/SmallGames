{
	"variables": {
		"WINDOW%": "glfw",
		"GPERFTOOLS%": "off",
		"PROFILER%": "off",
		"BUILD_DEBUG_SYMBOLS%": "off"
	},
	"targets": [
		{
			"target_name": "engine2D",
			"type": "executable",
			"dependencies": ["libengine2D"],
			"sources": [
				"src/main.cpp"
			],
			
			"conditions": [
				['OS == "win"', {
					'msvs_settings': {
						'VCLinkerTool': {
						
						},
					}
				}]
			]
		},
		{
			"target_name": "libengine2D",
			"type": "shared_library",
			"dependencies": [],
			"sources": [
				"src/stdlib.cpp",
				"src/Database.cpp",
				"src/Filesystem.cpp",
				"src/GL3Buffer.cpp",
				"src/Shader.cpp",
				"src/EngineUI.cpp",
				"src/Draw2D.cpp",
				"src/Draw3D.cpp",
				"src/RenderDriver.cpp",
				"src/RenderGL3.cpp",
				"src/Logger.cpp",
				"src/Profiler.cpp",
				"src/FramePerfMonitor.cpp",
				"src/ResourceManager.cpp",
				"src/Config.cpp",
				"src/Util.cpp",
				"src/Platform_mac.cpp",
				"src/Platform_win.cpp",
				"src/Platform_linux.cpp",
				"src/Events.cpp",
				"src/TestSuite.cpp",
				"src/Application.cpp",
				"src/PlatformTests.cpp",
				"src/CoreTests.cpp",
				"src/ScriptingTests.cpp",
				"src/StdLibTests.cpp",
				"src/PackageTests.cpp",
				"src/SpriteSheet.cpp",
				"src/FontSheet.cpp",
				"src/TextureLoader.cpp",
				"src/Timer.cpp",
				"src/ScriptingManager.cpp",
				"src/WorkerThreadPool.cpp",
				"src/Package.cpp",
				"src/Addon.cpp",

				# Script bindings
				"src/gen/JSInput.cpp",
				"src/gen/JSDraw.cpp",
				"src/gen/JSSys.cpp",
				"src/gen/JSFS.cpp",
				"src/gen/JSDatabase.cpp",
				"src/gen/JSMathHelper.cpp",
				"src/gen/JSHeadless.cpp",

				# Drawables
				"src/Drawables/CubeDrawableTest.cpp",

				# Misc vendor items
				"src/vendor/jsoncpp.cpp",
				"src/vendor/sqlite3.c",
				"src/vendor/glew.c",

				# SOIL
				"src/vendor/soil/image_DXT.c",
				"src/vendor/soil/image_helper.c",
				"src/vendor/soil/SOIL.c",
				"src/vendor/soil/stb_image_aug.c",

				# ZLIB
				"src/vendor/zlib123/adler32.c",
				"src/vendor/zlib123/compress.c",
				"src/vendor/zlib123/crc32.c",
				"src/vendor/zlib123/deflate.c",
				"src/vendor/zlib123/gzio.c",
				"src/vendor/zlib123/infback.c",
				"src/vendor/zlib123/inffast.c",
				"src/vendor/zlib123/inflate.c",
				"src/vendor/zlib123/inftrees.c",
				"src/vendor/zlib123/trees.c",
				"src/vendor/zlib123/uncompr.c",
				"src/vendor/zlib123/zutil.c",

				# PHYSFS
				"src/vendor/physfs/physfs.c",
				"src/vendor/physfs/physfs_byteorder.c",
				"src/vendor/physfs/physfs_unicode.c",
				"src/vendor/physfs/platform/posix.c",
				"src/vendor/physfs/platform/unix.c",
				"src/vendor/physfs/platform/macosx.c",
				"src/vendor/physfs/platform/windows.c",
				"src/vendor/physfs/archivers/dir.c",
				"src/vendor/physfs/archivers/zip.c",

				# SHA1
				"src/vendor/sha512.cpp",
				"src/vendor/sha256.cpp",

				# enet
				"src/vendor/enet/callbacks.c",
				"src/vendor/enet/compress.c",
				"src/vendor/enet/host.c",
				"src/vendor/enet/list.c",
				"src/vendor/enet/packet.c",
				"src/vendor/enet/peer.c",
				"src/vendor/enet/protocol.c",
				"src/vendor/enet/unix.c",
				"src/vendor/enet/win32.c"
			],
			"conditions": [
				['OS != "win"', {
					'sources!': [
						# Windows-only; exclude on other platforms.
						"src/Platform_win.cpp",
					]
				}],
				['OS != "mac"', {
					'sources!': [
						# Mac-only; exclude on other platforms.
						"src/Platform_mac.cpp",
					]
				}],
				['OS != "linux"', {
					'sources!': [
						# Mac-only; exclude on other platforms.
						"src/Platform_linux.cpp",
					]
				}],
				["OS == \"mac\"", {
					"xcode_settings": {
						"OTHER_CPLUSPLUSFLAGS": [
							"-D_FORTIFY_SOURCE=2",
							"-std=gnu++11",
							"-stdlib=libc++",
							"-pthread"
						],
						"OTHER_LDFLAGS": [
							"-lv8"
						]
					},
					"include_dirs": [
						"third_party/include"
					],
					"library_dirs": [
						"third_party/lib"	
					],
					"link_settings": {
						"libraries": [
							"$(SDKROOT)/System/Library/Frameworks/Cocoa.framework",
							"$(SDKROOT)/System/Library/Frameworks/OpenGL.framework",
							"$(SDKROOT)/System/Library/Frameworks/IOKit.framework"
						]
					}
				}],
				["OS == \"win\"", {
					"include_dirs": [
						"third_party/include"
					],
					"library_dirs": [
						"third_party/lib"	
					],
					'msvs_settings': {
						'VCCLCompilerTool': {
							"ObjectFile": "$(IntDir)//%(RelativeDir)/"
						},
						'VCLinkerTool': {
						
						},
						'VCResourceCompilerTool': {
							
						},
					},
					"libraries": [
						"kernel32.lib",
						"user32.lib",
						"gdi32.lib",
						"comdlg32.lib",
						"advapi32.lib",
						"shell32.lib",
						"ole32.lib",
						"uuid.lib",
						"v8.native.lib",
						"opengl32.lib",
						"Ws2_32.lib",
						"winmm.lib",
						"Rpcrt4.lib"
					]
				}],
				["OS == \"linux\"", {
					"cflags": [
						"-fPIC"
					],
					"cppflags": [
						"-D_FORTIFY_SOURCE=2",
						"-std=gnu++11",
						"-stdlib=libc++",
						"-pthread",
						"-fPIC"
					],
					"ldflags": [
						"-lv8",
						"-lGL",
						"-luuid",
						"-L../third_party/lib",
						"-Wl,-rpath,../third_party/lib"
					],
					"include_dirs": [
						"third_party/include"
					],
					"library_dirs": [
						"third_party/lib"	
					]
				}],
				['("<(WINDOW)" == "glfw") & (OS == "linux")', {
					'sources': [
						"src/Window_glfw.cpp"
					],
					"ldflags": [
						"-lglfw"
					]
				}],
				['("<(WINDOW)" == "sdl") & (OS == "linux")', {
					'sources': [
						"src/Window_sdl.cpp"
					],
					"cppflags": [
						"-D_THREAD_SAFE",
						"-D_REENTRANT"
					],
					"ldflags": [
						"-lSDL2"
					],
					"include_dirs": [
						"/usr/include/SDL2"
					]
				}],
				['("<(WINDOW)" == "glfw") & (OS == "mac")', {
					'sources': [
						"src/Window_glfw.cpp"
					],
					"xcode_settings": {
						"OTHER_LDFLAGS": [
							"-lglfw"
						]
					}
				}],
				['("<(WINDOW)" == "sdl") & (OS == "mac")', {
					'sources': [
						"src/Window_sdl.cpp"
					],
					"xcode_settings": {
						"OTHER_CPLUSPLUSFLAGS": [
							"-D_THREAD_SAFE"
						],
						"OTHER_LDFLAGS": [
							"-lSDL2"
						]
					},
					"include_dirs": [
						"/usr/local/include/SDL2"
					]
				}],
				['("<(WINDOW)" == "glfw") & (OS == "win")', {
					'sources': [
						"src/Window_glfw.cpp"
					],
					"libraries": [
						"glfw3dll.lib"
					]
				}],
				['("<(GPERFTOOLS)" == "on") & (OS == "mac")', {
					"xcode_settings": {
						"OTHER_LDFLAGS": [
							"-ltcmalloc",
							"-lprofiler"
						]
					}
				}],
				['("<(PROFILER)" == "on") & (OS == "mac")', {
					"xcode_settings": {
						"OTHER_CPLUSPLUSFLAGS": [
							"-DPROFILER"
						]
					}
				}],
				['("<(PROFILER)" == "on") & (OS == "win")', {
					"defines": [
						"PROFILER"
					]
				}],
				['("<(BUILD_DEBUG_SYMBOLS)" != "off") & (OS == "win")', {
					"msvs_settings": {
						"VCLinkerTool": {
							"GenerateDebugInformation": "true"
						}
					}
				}]
			]
		}
	]
}
