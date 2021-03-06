/*
   Filename: Application.cpp
   Purpose:  Main application logic and management

   Part of Engine2D

   Copyright (C) 2014 Vbitz

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "Application.hpp"

#include <cstring>

#include "vendor/json/json.h"
#include "vendor/soil/SOIL.h"

#include "Config.hpp"
#include "Profiler.hpp"

#include "ResourceManager.hpp"

#include "TestSuite.hpp"

#include "FramePerfMonitor.hpp"
#include "Timer.hpp"

#include "PlatformTests.hpp"
#include "CoreTests.hpp"
#include "ScriptingTests.hpp"
#include "StdLibTests.hpp"
#include "PackageTests.hpp"

#include "EngineUI.hpp"

#include "Build.hpp"

namespace Engine {
    
    bool _hasGLContext = false;
    
    bool HasGLContext() {
        return _hasGLContext;
    }
    
    void EnableGLContext() {
        _hasGLContext = true;
    }
    
    void DisableGLContext() {
        _hasGLContext = false;
    }
	
	void Application::_updateMousePos() {
        glm::vec2 mouse = _window->GetCursorPos();
        
        ScriptingManager::Factory f(this->_scripting->GetIsolate());
        
        f.FillObject(this->_scripting->GetScriptTable("input"), {
            {FTT_Static, "mouseX", f.NewNumber(mouse.x)},
            {FTT_Static, "mouseY", f.NewNumber(mouse.y)},
            {FTT_Static, "leftMouseButton", f.NewBoolean(this->_window->GetMouseButtonPressed(MouseButton_Left))},
            {FTT_Static, "middleMouseButton", f.NewBoolean(this->_window->GetMouseButtonPressed(MouseButton_Middle))},
            {FTT_Static, "rightMouseButton", f.NewBoolean(this->_window->GetMouseButtonPressed(MouseButton_Right))},
        });
	}
	
	void Application::UpdateScreen() {
        if (!this->_running) {
            return;
        }
        
        ScriptingManager::Factory f(this->_scripting->GetIsolate());
        
        glm::vec2 size = this->_window->GetWindowSize();
        
        f.FillObject(this->_scripting->GetScriptTable("sys"), {
            {FTT_Static, "screenWidth", f.NewNumber(size.x)},
            {FTT_Static, "screenHeight", f.NewNumber(size.y)}
        });
	}
    
    void Application::_updateFrameTime() {
        ScriptingManager::Factory f(this->_scripting->GetIsolate());
        
        f.FillObject(this->_scripting->GetScriptTable("sys"), {
            {FTT_Static, "deltaTime", f.NewNumber(FramePerfMonitor::GetFrameTime())}
        });
    }
    
    void Application::_disablePreload() {
        ScriptingManager::Factory f(this->_scripting->GetIsolate());
        
        if (!this->IsHeadlessMode()) {
            this->_scripting->SetScriptTableValue("draw", {FTT_Hidden, "_draw", f.NewExternal(new Draw2D(GetRender()))});
        }
        
        this->_scripting->SetScriptTableValue("sys", {FTT_Static, "preload", f.NewBoolean(false)});
    }
    
    void Application::_loadBasicConfigs() {
        // Core
        Config::SetBoolean( "core.runOnIdle",                       false);
        Config::SetBoolean( "core.throttleOnIdle",                  true);
        Config::SetBoolean( "core.catchErrors",                     !this->_debugMode && !this->_testMode);

        // Modes
        Config::SetBoolean( "core.debugMode",                       this->_debugMode);
        Config::SetBoolean( "core.devMode",                         this->_developerMode);
        Config::SetBoolean( "core.testMode",                        this->_testMode);
        
        // Window
        Config::SetNumber(  "core.window.width",                    800);
        Config::SetNumber(  "core.window.height",                   600);
        Config::SetBoolean( "core.window.vsync",                    false); // lack of vsync causes FPS issues
        Config::SetBoolean( "core.window.fullscreen",               false);
        Config::SetString(  "core.window.title",                    "Engine2D");
        
        // Render
        Config::SetNumber(  "core.render.aa",                       4);
        Config::SetString(  "core.render.openGL",                   "3.2");
        Config::SetString(  "core.render.basicEffect",              "shaders/basic.json");
        Config::SetNumber(  "core.render.targetFrameTime",          1.0f / 30.0f);
        Config::SetBoolean( "core.render.clampTexture",             true);
        Config::SetBoolean( "core.render.forceMipmaps",             true);
        Config::SetNumber(  "core.render.fovy",                     45.0f);
        Config::SetBoolean( "core.render.halfPix",                  false);

        // Content
        Config::SetString(  "core.content.fontPath",                "fonts/open_sans.json");

        // Debug
        Config::SetBoolean( "core.debug.engineUI.showVerboseLog",   false);
        Config::SetBoolean( "core.debug.engineUI",                  this->_developerMode);
        Config::SetNumber(  "core.debug.engineUI.profilerScale",    500);
        Config::SetBoolean( "core.debug.profiler",                  this->_developerMode || this->_debugMode);
        Config::SetNumber(  "core.debug.profiler.maxFrameTime",     1.0f / 50.0f);
        Config::SetBoolean( "core.debug.profiler.dumpFrames",       this->_debugMode);
        Config::SetNumber(  "core.debug.profiler.dumpCooldown",     100);
        Config::SetBoolean( "core.debug.debugRenderer",             true);
        Config::SetBoolean( "core.debug.v8Debug",                   this->_developerMode);
        Config::SetNumber(  "core.debug.v8Debug.port",              5858);
        Config::SetBoolean( "core.debug.slowload",                  false);
        
        // Script
        Config::SetBoolean( "core.script.autoReload",               this->_developerMode);
#ifdef _PLATFORM_WIN32
        Config::SetBoolean( "core.script.gcOnFrame",                false);
#else
		Config::SetBoolean( "core.script.gcOnFrame",				this->_developerMode);
#endif
        Config::SetString(  "core.script.loader",                   "lib/boot.js");
        Config::SetString(  "core.script.entryPoint",               "script/basic");

        // Config
        Config::SetString(  "core.config.path",                     "config/config.json");

        // Test
        Config::SetNumber(  "core.test.testFrames",                 0);
        Config::SetNumber(  "core.test.screenshotTime",             0);
        
        // Log
        // With quite a bit of research into console logging performance on windows it seems like I should be using
        // printf or buffered std::cout
        Config::SetBoolean( "core.log.enableConsole",               true);
        Config::SetBoolean( "core.log.filePath",                    "");
        Config::SetBoolean( "core.log.levels.verbose",              this->_developerMode || this->_debugMode);
        Config::SetBoolean( "core.log.levels.onlyHighlight",        false);
        Config::SetBoolean( "core.log.showColors",                  true);
        Config::SetBoolean( "core.log.src.undefinedValue",          this->_developerMode);
        Config::SetBoolean( "core.log.src.perfIssues",              this->_developerMode);
        Config::SetBoolean( "core.log.src.createImage",             true);
    }
    
    EventMagic Application::_config_CoreRenderAA(Json::Value args, void* userPointer) {
        static_cast<ApplicationPtr>(userPointer)->GetWindow()->SetAntiAlias(Config::GetInt("core.render.aa"));
        return EM_OK;
    }
    
    EventMagic Application::_config_CoreWindowVSync(Json::Value args, void* userPointer) {
        static_cast<ApplicationPtr>(userPointer)->GetWindow()->SetVSync(Config::GetBoolean("core.window.vsync"));
        return EM_OK;
    }
    
    EventMagic Application::_config_CoreWindowSize(Json::Value args, void* userPointer) {
        static_cast<ApplicationPtr>(userPointer)->GetWindow()->SetWindowSize(glm::vec2(Config::GetInt("core.window.width"), Config::GetInt("core.window.height")));
        return EM_OK;
    }
    
    EventMagic Application::_config_CoreWindowTitle(Json::Value args, void* userPointer) {
        static_cast<ApplicationPtr>(userPointer)->GetWindow()->SetTitle(Config::GetString("core.window.title"));
        return EM_OK;
    }
    
    void Application::_hookConfigs() {
        if (IsHeadlessMode()) return;
        EventEmitterPtr eventsSingilton = GetEventsSingilton();

        eventsSingilton->GetEvent("config:core.render.aa")->AddListener("Application::Config_CoreRenderAA", EventEmitter::MakeTarget(_config_CoreRenderAA, this));
        eventsSingilton->GetEvent("config:core.window.vsync")->AddListener("Application::Config_CoreWindowVSync", EventEmitter::MakeTarget(_config_CoreWindowVSync, this));
        eventsSingilton->GetEvent("config:core.window.width")->AddListener("Application::ConfigWindowSize_Width", EventEmitter::MakeTarget(_config_CoreWindowSize, this));
        eventsSingilton->GetEvent("config:core.window.height")-> AddListener("Application::ConfigWindowSize_Height", EventEmitter::MakeTarget(_config_CoreWindowSize, this));
        eventsSingilton->GetEvent("config:core.window.title")->AddListener("Application::Config_CoreWindowTitle", EventEmitter::MakeTarget(_config_CoreWindowTitle, this));
    }
    
    void Application::_loadConfigFile(std::string configPath) {
        Logger::begin("Application", Logger::LogLevel_Log) << "Loading Config: " << configPath << Logger::end();
        Json::Value root;
        Json::Reader reader;
        bool fileLoaded = reader.parse(Filesystem::GetFileContent(configPath), root);
        if (!fileLoaded) {
            Logger::begin("Application", Logger::LogLevel_Error) << "Failed to load Config" << Logger::end();
            return;
        }
        if (!root.isObject()) {
            Logger::begin("Application", Logger::LogLevel_Error) << "Config root has to be a object" << Logger::end();
            return;
        }
        if (root.empty()) {
            return;
        }
        Json::Value::Members keys = root.getMemberNames();
        for (auto iter = keys.begin(); iter != keys.end(); iter++) {
            Json::Value value = root[*iter];
            if (value.isNull()) {
                // ignore null values
            } else if (value.isString()) {
                Config::SetString(*iter, value.asString());
            } else if (value.isNumeric()) {
                Config::SetNumber(*iter, value.asFloat());
            } else if (value.isBool()) {
                Config::SetBoolean(*iter, value.asBool());
            }
        }
    }
    
    void Application::_printConfigVars() {
        Logger::begin("Application", Logger::LogLevel_Log) << "== Config Vars ==" << Logger::end();
        std::vector<std::string> items = Config::GetAll();
        for (auto iter = items.begin(); iter != items.end(); iter++) {
            Logger::begin("Config", Logger::LogLevel_Log) << *iter << Logger::end();
        }
    }
    
    EventMagic Application::_appEvent_Exit(Json::Value val, void* userPointer) {
        static_cast<ApplicationPtr>(userPointer)->Exit();
        return EM_OK;
    }
    
    EventMagic Application::_appEvent_DumpScripts(Json::Value args, void* userPointer) {
        static_cast<ApplicationPtr>(userPointer)->DumpScripts();
        return EM_OK;
    }
    
    void Application::_hookEvents() {
        EventEmitterPtr eventsSingilton = GetEventsSingilton();
        
        eventsSingilton->GetEvent("exit")->AddListener("Application::_appEvent_Exit", EventEmitter::MakeTarget(_appEvent_Exit, this));
        eventsSingilton->GetEvent("dumpScripts")->AddListener("Applicaton::_appEvent_DumpScripts", EventEmitter::MakeTarget(_appEvent_DumpScripts, this));
        
        eventsSingilton->GetEvent("toggleFullscreen")->AddListener("Application::_toggleFullscreen", EventEmitter::MakeTarget(_toggleFullscreen, this))->SetDefered(true);
        eventsSingilton->GetEvent("restartRenderer")->AddListener("Application::_restartRenderer", EventEmitter::MakeTarget(_restartRenderer, this))->SetDefered(true);
        eventsSingilton->GetEvent("screenshot")->AddListener("Application::_saveScreenshot", EventEmitter::MakeTarget(_saveScreenshot, this))->SetDefered(true);
        eventsSingilton->GetEvent("dumpLog")->AddListener("Application::_dumpLog", EventEmitter::MakeTarget(_dumpLog, this));
        
        eventsSingilton->GetEvent("runFile")->AddListener(10, "Application::_requireDynamicLibary", EventEmitter::MakeTarget(_requireDynamicLibary, this));
        eventsSingilton->GetEvent("runFile")->AddListener(10, "Application::_requireConfigFile", EventEmitter::MakeTarget(_requireConfigFile, this));
    }
    
    void Application::_initGLContext(GraphicsVersion v) {
        this->_renderGL = this->_window->GetRender();
        
        this->_renderGL->Set(RenderStateFlag::Blend, true);
        this->_renderGL->Set(RenderStateFlag::DepthTest, true);
        
        if (v == GraphicsVersion::OpenGL_Legacy) {
            this->_renderGL->Set(RenderStateFlag::Lighting, false);
        }
        
        this->GetRender()->CheckError("PostSetupContext");
        
        Logger::begin("Window", Logger::LogLevel_Highlight) << "Loaded OpenGL" << Logger::end();

        this->GetRender()->Init2d();
    }
    
    void Application::_openWindow(int width, int height, bool fullscreen, std::string openGL3Context) {
        Logger::begin("Window", Logger::LogLevel_Log) << "Loading OpenGL : Init Window/Context" << Logger::end();
        
        GraphicsVersion v;
        
        if (openGL3Context == "3.2") {
            v = GraphicsVersion::OpenGL_Modern;
        } else {
            v = GraphicsVersion::OpenGL_Legacy;
        }
        
        this->_window = CreateWindow(v);
        
        this->_window->SetWindowSize(glm::vec2(width, height));
        this->_window->SetTitle(Config::GetString("core.window.title"));
        this->_window->SetFullscreen(fullscreen);
        this->_window->SetAntiAlias(Config::GetInt("core.render.aa"));
        this->_window->SetDebug(Config::GetBoolean("core.debug.debugRenderer"));
        
        this->_window->Show();
        
        Logger::begin("Window", Logger::LogLevel_Verbose) << "Loading OpenGL : Init OpenGL State" << Logger::end();
        
        this->GetRender()->CheckError("PostSetCallback");
    }
    
    void Application::_closeWindow() {
        Logger::begin("Window", Logger::LogLevel_Log) << "Terminating Window" << Logger::end();
        delete this->_window;
        this->_window = NULL;
    }
    
    EventMagic Application::_rawInputHandler(Json::Value val, void* userPointer) {
        ApplicationPtr app = static_cast<ApplicationPtr>(userPointer);
        
        app->_engineUI->OnKeyPress(val["rawKey"].asInt(), val["rawPress"].asInt(), val["shift"].asBool());
        
        if (app->_engineUI->ConsoleActive()) {
            return EM_OK;
        }
        
        std::string key = val["key"].asString();
        
        GetEventsSingilton()->GetEvent("key_" + key)->Emit(val);
        GetEventsSingilton()->GetEvent("input")->Emit(val);
        
        return EM_OK;
    }
    
    EventMagic Application::_rendererKillHandler(Json::Value val, void* userPointer) {
        Logger::begin("Window", Logger::LogLevel_Verbose) << "Window Destroyed" << Logger::end();
        ResourceManager::UnloadAll();
        return EM_OK;
    }
    
    EventMagic Application::_postCreateContext(Json::Value val, void* userPointer) {
        ApplicationPtr app = static_cast<ApplicationPtr>(userPointer);
        app->_initGLContext(app->_window->GetGraphicsVersion());
        return EM_OK;
    }
    
    EventMagic Application::_rawResizeHandler(Json::Value val, void* userPointer) {
        static_cast<ApplicationPtr>(userPointer)->UpdateScreen();
        return EM_OK;
    }
	
	void Application::_initOpenGL() {
        Logger::begin("Window", Logger::LogLevel_Verbose) << "Loading OpenGL : Init Window" << Logger::end();
        
        GetEventsSingilton()->GetEvent("destroyWindow")->AddListener("Application::RendererKillHandler", EventEmitter::MakeTarget(_rendererKillHandler, this));
        GetEventsSingilton()->GetEvent("rawInput")->AddListener("Application::RawInputHandler", EventEmitter::MakeTarget(_rawInputHandler, this));
        GetEventsSingilton()->GetEvent("postCreateContext")->AddListener("Application::_postCreateContext", EventEmitter::MakeTarget(_postCreateContext, this));
        GetEventsSingilton()->GetEvent("rawResize")->AddListener("Application::RawResizeHandler", EventEmitter::MakeTarget(_rawResizeHandler, this));
        
        Window::StaticInit();
        
        this->_openWindow(Config::GetInt("core.window.width"), Config::GetInt("core.window.height"),
                   Config::GetBoolean("core.window.fullscreen"), Config::GetString("core.render.openGL"));
	}
	
	void Application::_shutdownOpenGL() {
        ResourceManager::UnloadAll();
        this->_closeWindow();
        Window::StaticDestroy();
	}
    
    // command line handlers
    
    void Application::_applyDelayedConfigs() {
        for (auto iter = _delayedConfigs.begin(); iter != _delayedConfigs.end(); iter++) {
            if (!Config::Set(iter->first, iter->second)) {
                Logger::begin("CommandLine", Logger::LogLevel_Error) << "Could not set '"
                << iter->first << "' to '" << iter->second
                << "' Ignoring" << Logger::end();
            }
        }
    }
    
    // command line parsing
    
    bool Application::_parseCommandLine(int argc, const char* argv[]) {
        bool _exit = false;
        
        for (int i = 1; i < argc; i++) {
            std::string arg = std::string(argv[i]);
            
            if (arg == "-devmode") {
                // enable devmode
                _developerMode = true;
            } else if (arg == "-debug") {
                // enable debug
                _debugMode = true;
            } else if (arg == "-test") {
                // start test mode
                _testMode = true;
            } else if (arg == "-headless") {
                // enable headless mode
                _headlessMode = true;
            } else if (arg == "-v8-options") {
                ScriptingManager::Context::RunHelpCommand();
                _exit = true; // v8 exits automaticly but let's just help it along
            } else if (arg == "-Cvars") {
                _configVarsMode = true;
            } else if (arg == "-h" | arg == "-help") {
                std::cerr << "Engine2D\n"
                "==================\n"
                "\n"
                "-- Examples --\n"
                "bin/Engine - loads config/config.json and sets basic config values\n"
                "bin/Engine -Ccore.window.width=1280 -Ccore.window.height=800 - loads config/config.json and "
                "overrides the basic configs core.window.width and core.window.height\n"
                "bin/Engine -config=config/test.json - loads config/test.json and sets basic config values\n"
                "bin/Engine script/test - any non `-` configs are passed onto javascript\n"
                "\n"
                "-- Args --\n"
                "-Cname=value                   - Overloads a basic config. This is applyed before loading the basic config"
                " but overrides those configs while they are applyed.\n"
                "-Cvars                         - Prints all Config Variables megred from the defaults and the set config file.\n"
                "-mountPath=archiveFile         - Loads a archive file using PhysFS, this is applyed after physfs is started.\n"
                "-test                          - Runs the built in test suite.\n"
                "-headless                - Loads scripting without creating a OpenGL context, any calls requiring OpenGL"
                " will fail.\n"
                "-devmode                       - Enables developer mode (This enables real time script loading and the console).\n"
                "-debug                         - Enables debug mode (This enables a OpenGL debug context and will print messages"
                "to the console).\n"
                "(NYI) -log=filename            - Logs logger output to filename (This is not writen using PhysFS so it needs a "
                "regular path).\n"
                "-v8-options                    - Prints v8 option help then exits.\n"
                "-v8flag=value                  - Set's v8 flags from the command line.\n"
                "(NYI) -recordReplay=filename   - Records a replay and saves it to json file\n"
                "(NYI) -playReplay=filename     - Plays back a replay\n"
                "-h                             - Prints this message.\n"
                "" << std::endl;
                _exit = true;
            } else if (arg.find("-C") == 0) {
                // set delayed config
                std::string key = arg.substr(2, arg.find("=") - 2);
                std::string value = arg.substr(arg.find("=") + 1);
                
                _delayedConfigs[key] = std::string(value);
            } else if (arg.find("-v8") == 0) {
                // set delayed config
                std::string key = arg.substr(3);
                
                Logger::begin("Scripting_CFG", Logger::LogLevel_Log) << "Setting V8 Option: --" << key << Logger::end();
                
                ScriptingManager::Context::SetFlag("--" + key);
            } else if (arg.find("-mountPath=") == 0) {
                // add archive to PhysFS after PhysFS Init
                this->_archivePaths.push_back(arg.substr(11));
            } else if (arg.find("-log=") == 0) {
                // set logfile and enable logging
            } else {
                // push to JS args
                _jsArgs.push_back(argv[i]);
            }
        }
        
        return !_exit;
    }
    
    // Test Suite Loading
	
    void Application::_loadTests() {
        if (_debugMode) {
            TestSuite::LoadTestSuiteTests();
        }
        LoadPlatformTests();
        LoadCoreTests();
        LoadScriptingTests();
        LoadStdLibTests();
        LoadPackageTests();
    }
	
	// public methods
    
    bool Application::IsDelayedConfig(std::string configKey) {
        return this->_delayedConfigs.count(configKey) != 0;
    }
    
    std::vector<std::string> Application::GetCommandLineArgs() {
        return _jsArgs;
    }
    
    void Application::Exit() {
        this->_running = false;
    }
    
    EventMagic Application::_restartRenderer(Json::Value args, void* userPointer) {
        ApplicationPtr app = static_cast<ApplicationPtr>(userPointer);
        Logger::begin("Window", Logger::LogLevel_Log) << "Restarting renderer" << Logger::end();
        app->_window->Reset();
        app->_initGLContext(app->_window->GetGraphicsVersion());
        app->UpdateScreen();
        return EM_OK;
    }
    
    EventMagic Application::_toggleFullscreen(Json::Value args, void* userPointer) {
        ApplicationPtr app = static_cast<ApplicationPtr>(userPointer);
        app->_window->SetFullscreen(!app->_window->GetFullscreen());
        app->UpdateScreen();
        return EM_OK;
    }
    
    EventMagic Application::_saveScreenshot(Json::Value args, void* userPointer) {
        ApplicationPtr app = static_cast<ApplicationPtr>(userPointer);
        std::string targetFilename = args["filename"].asString();
        
        int width, height;
        
        width = app->_window->GetWindowSize().x;
        height = app->_window->GetWindowSize().y;
        
        Filesystem::TouchFile(targetFilename);
        
        SOIL_save_screenshot(Filesystem::GetRealPath(targetFilename).c_str(), SOIL_SAVE_TYPE_BMP, 0, 0, width, height);
        
        Logger::begin("Screenshot", Logger::LogLevel_Log) << "Saved Screenshot as: " << Filesystem::GetRealPath(targetFilename) << Logger::end();
        
        Json::Value saveArgs(Json::objectValue);
        
        saveArgs["filename"] = Filesystem::GetRealPath(targetFilename);
        
        GetEventsSingilton()->GetEvent("onSaveScreenshot")->Emit(saveArgs);
        
        return EM_OK;
    }
    
    EventMagic Application::_dumpLog(Json::Value args, void* userPointer) {
        std::string targetFilename = args["filename"].asString();
        std::string log = Logger::DumpAllEvents();
        
        Filesystem::WriteFile(targetFilename, log.c_str(), log.length());
        
        Logger::begin("DumpLog", Logger::LogLevel_Log) << "Saved Logfile as: " << Filesystem::GetRealPath(targetFilename) << Logger::end();
        
        Json::Value saveArgs(Json::objectValue);
        
        saveArgs["filename"] = Filesystem::GetRealPath(targetFilename);
        
        GetEventsSingilton()->GetEvent("onSaveLog")->Emit(saveArgs);
        
        return EM_OK;
    }
    
    EventMagic Application::_requireDynamicLibary(Json::Value args, void* userPointer) {
        static size_t endingLength = strlen(_PLATFORM_DYLINK);
        std::string filename = args["path"].asString();
        if (0 == filename.compare(filename.length() - endingLength, endingLength, _PLATFORM_DYLINK)) {
            Logger::begin("Application::_requireDynamicLibary", Logger::LogLevel_Verbose) << "Loading shared library: " << filename << Logger::end();
            Platform::LibaryPtr lib = Platform::OpenLibary(Filesystem::GetRealPath(filename));
            if (!lib->IsValid()) {
                Logger::begin("Application::_requireDynamicLibary", Logger::LogLevel_Error) << "Error loading shared library: " << filename << Logger::end();
            }
            return EM_CANCEL;
        } else {
            return EM_OK;
        }
    }
    
    EventMagic Application::_requireConfigFile(Json::Value args, void* userPointer) {
        static size_t endingLength = strlen(".json");
        std::string filename = args["path"].asString();
        if (0 == filename.compare(filename.length() - endingLength, endingLength, ".json")) {
            static_cast<ApplicationPtr>(userPointer)->_loadConfigFile(filename);
            return EM_CANCEL;
        } else {
            return EM_OK;
        }
    }
    
    bool Application::GetKeyPressed(int key) {
        return this->_window->GetKeyStatus(key) == Key_Press;
    }
    
    std::string Application::GetEngineVersion() {
        return std::string("Engine2D v ") + std::string(ENGINE_VERSION);
    }
    
    void Application::Assert(bool value, std::string reason, std::string line, int lineNumber) {
        if (value || !this->_debugMode) return;
        Logger::begin("Assert", Logger::LogLevel_Error) << "ASSERT FAILED : (" << line << ":" << lineNumber << ") " << reason << Logger::end();
        Platform::DumpStackTrace();
        throw "Assert Failed";
        //exit(EXIT_FAILURE); // Can't recover from this
    }
    
    void Application::AddScript(const char* filename_str, size_t filename_len) {
		this->_pendingScripts.push(RawScriptInfo(filename_str, filename_len ));
    }
    
    void Application::DumpScripts() {
        Logger::begin("Application::DumpScripts", Logger::LogLevel_Log) << "Starting Application::DumpScripts" << Logger::end();
        for (auto iter = this->_scripts.begin(); iter != this->_scripts.end(); iter++) {
            Logger::begin("Application::DumpScripts", Logger::LogLevel_Log) << "    " << iter->second.filename << Logger::end();
        }
    }
    
    // Modifyed slightly from node.js at https://github.com/joyent/node/blob/master/src/node_win32_etw_provider.cc
    
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
    
    struct v8tags {
        char prefix[32 - sizeof(size_t)];
        size_t prelen;
    };
    
    // The v8 CODE_ADDED event name has a prefix indicating the type of event.
    // Many of these are internal to v8.
    // The trace_codes array specifies which types are written.
    struct v8tags trace_codes[] = {
#define MAKE_V8TAG(s) { s, sizeof(s) - 1 }
        MAKE_V8TAG("LazyCompile:"),
        MAKE_V8TAG("Script:"),
        MAKE_V8TAG("Function:"),
        MAKE_V8TAG("RegExp:"),
        MAKE_V8TAG("Eval:")
#undef MAKE_V8TAG
    };
    
    // v8 sometimes puts a '*' or '~' in front of the name.
#define V8_MARKER1 '*'
#define V8_MARKER2 '~'
    
    
    // If prefix is not in filtered list return -1,
    // else return length of prefix and marker.
    int _filterCodeEvents(const char* name, size_t len) {
        for (int i = 0; i < ARRAY_SIZE(trace_codes); i++) {
            size_t prelen = trace_codes[i].prelen;
            if (prelen < len) {
                if (strncmp(name, trace_codes[i].prefix, prelen) == 0) {
                    if (name[prelen] == V8_MARKER1 || name[prelen] == V8_MARKER2 || name[prelen] == ' ')
                        prelen++;
                    return prelen;
                }
            }
        }
        return -1;
    }

    
    void Application::_processScripts() {
        while (this->_pendingScripts.size() > 0) {
            RawScriptInfo i = this->_pendingScripts.front();
            
            int prefixLength = _filterCodeEvents(i.filename_str, i.filename_len);
            if (prefixLength >= 0) {
                std::string filename = std::string(i.filename_str + prefixLength, i.filename_len - prefixLength);
                if (filename.length() > 0) {
                    if (this->_scripts.count(filename) > 0) {
                        // update infomation
                    } else {
                        // add new
                        this->_scripts[filename] = ScriptInfo(filename);
                    }
                }
            }
            
            this->_pendingScripts.pop();
        }
    }
    
    void Application::_updateAddonLoad(LoadOrder load) {
        Addon::LoadAll(load);
        this->_currentLoadingState = load;
    }
    
    void Application::_mainLoop() {
        this->_running = true;
        
		while (this->_running) {
            
            if (!this->_window->ShouldClose() &&  // Check to make sure were not going to close
                !this->_window->IsFocused()) { // Check to make sure were not focused
                if (!this->_window->GetFullscreen() && // Check to make sure were not in fullscreen mode
                    !Config::GetBoolean("core.runOnIdle")) {
                    double startPauseTime = Platform::GetTime();
                    this->_window->WaitEvents();
                    Platform::Sleep(0);
                    // notify the timer that it needs to offset the values to keep time acurate for user interation and physics
                    Timer::NotifyPause(Platform::GetTime() - startPauseTime);
                    continue;
                } else {
                    if (Config::GetBoolean("core.throttleOnIdle") && !this->_window->GetFullscreen()) {
                        Platform::NanoSleep(150000);
                    }
                }
            }
            
            RenderDriverPtr render = this->GetRender();
            ScriptingManager::Factory f(this->GetScriptingContext()->GetIsolate());
            
            Profiler::BeginProfileFrame();
            
            Engine::Profiler::Scope profilerScope("FrameScope");
            
            FramePerfMonitor::BeginFrame();
            Timer::Update(); // Timer events may be emited now, this is the soonest into the frame that Javascript can run
            GetEventsSingilton()->PollDeferedMessages(); // Events from other threads will run here by default, Javascript may run at this time
            this->_processScripts();
            
			this->_scripting->CheckUpdate();
            
            this->_updateFrameTime();
            
            this->_updateMousePos();
            
            render->CheckError("startOfRendering");
            
            render->Clear();
            
            FramePerfMonitor::BeginDraw();
			{
                Engine::Profiler::Scope profilerScopeDraw("DrawScope");
            
                render->Begin2d();
            
                v8::Handle<v8::Value> args[1] = {
                    f.NewNumber(FramePerfMonitor::GetFrameTime())
                };
                GetEventsSingilton()->GetEvent("draw")->Emit(Json::nullValue, 1, args); // this is when most Javascript runs
            
                render->End2d();
            
                render->Begin2d();
            
                //this->_cubeTest->Draw();
            
                this->_engineUI->Draw();
            
                render->End2d();
                
				profilerScopeDraw.Close();
            }
            FramePerfMonitor::EndDraw();

            this->_window->Present();
            
			GetEventsSingilton()->GetEvent("endOfFrame")->Emit();
            
			if (this->_window->ShouldClose()) {
				Logger::begin("Window", Logger::LogLevel_Log) << "Exiting" << Logger::end();
				this->_running = false;
				break;
			}
            
            render->CheckError("endOfRendering");
            
            if (Config::GetBoolean("core.script.gcOnFrame")) {
                this->GetScriptingContext()->TriggerGC();
			}
            
            GetEventsSingilton()->PollDeferedMessages("toggleFullscreen");
            GetEventsSingilton()->PollDeferedMessages("restartRenderer");
            GetEventsSingilton()->PollDeferedMessages("screenshot");
            GetEventsSingilton()->PollDeferedMessages("dumpProfile");
            
            if (this->_frames == 0) {
                this->_updateAddonLoad(LoadOrder::FirstFrame);
                if (!this->_testMode) this->_frames++;
            }
            
            if (this->_testMode && this->_frames++ > Config::GetInt("core.test.testFrames")) {
                this->Exit();
            }
            
            profilerScope.Close();
            
            this->GetRender()->EndFrame();
            FramePerfMonitor::EndFrame();
            Profiler::EndProfileFrame();
        }
    }
    
    void Application::_mainLoopHeadless() {
        this->_running = true;
        
        while (this->_running) {
            Timer::Update(); // Timer events may be emited now, this is the soonest into the frame that Javascript can run
            
            GetEventsSingilton()->GetEvent("headlessLoop")->Emit();
        }
    }
    
	// main function
    
    int Application::_postStart() {
        Logger::begin("Application", Logger::LogLevel_Log) << "Starting: " << Application::GetEngineVersion() << Logger::end();
        
        if (this->_debugMode) {
            Logger::begin("Application", Logger::LogLevel_Warning) << "=== Debug Mode Active ===" << Logger::end();
        }
        
        if (this->_developerMode) {
            Logger::begin("Application", Logger::LogLevel_Warning) << "=== Developer Mode Active ===" << Logger::end();
        }
        
        if (this->_headlessMode) {
            Logger::begin("Application", Logger::LogLevel_Warning) << "=== Headless Mode Active ===" << Logger::end();
        }
        
		Filesystem::Init(Platform::GetRawCommandLineArgV()[0]);
        
        Config::EnableConfigEvents();
        Logger::EnableLoggerEvents();
        this->_hookEvents();
        // The events system is now ready
        
        for (auto iter = this->_archivePaths.begin(); iter != this->_archivePaths.end(); iter++) {
            Filesystem::Mount(*iter);
        }
        
        // The filesystem is now ready for reading files
        
        this->_loadConfigFile(Config::GetString("core.config.path"));
        
        this->_applyDelayedConfigs();
        
        // Configuration is now setup
        
        if (this->_configVarsMode) {
            this->_printConfigVars();
            Filesystem::Destroy();
            return 0;
        }
        
        this->_updateAddonLoad(LoadOrder::PreScript);
        
        ScriptingManager::Context::StaticInit();
        this->_scripting = new ScriptingManager::Context();
        this->_scripting->InitScripting();
        
        // Scripting has now initalized, Javascript may punch in during any event
        
        this->_updateAddonLoad(LoadOrder::PreGraphics);
        
        if (!this->IsHeadlessMode()) {
            this->_initOpenGL();
        
            this->_engineUI = new EngineUI(this);
            
            // The window is now ready
        
            RenderCheckError("Post InitOpenGL");
        
            Engine::EnableGLContext();
        }
        
        this->_disablePreload();
        
        GetEventsSingilton()->GetEvent("postLoad")->Emit();
        
        RenderCheckError("On JS Post Load");
        
        //this->_cubeTest = this->_renderGL->CreateDrawable<Drawables::CubeDrawableTest>();
        
        if (!this->IsHeadlessMode()) {
            this->UpdateScreen();
        }
        
        RenderCheckError("Post Finish Loading");
        
        this->_updateAddonLoad(LoadOrder::Loaded);
        
        Logger::begin("Application", Logger::LogLevel_Highlight) << "Loaded" << Logger::end();
        
        this->_hookConfigs(); // this is the last stage of the boot process so previous code does'nt interfere.
        
        if (this->_testMode) {
            this->_loadTests();
            TestSuite::Run();
            if (Config::GetInt("core.test.testFrames") > 0) {
                if (!IsHeadlessMode()) {
                    this->_mainLoop();
                } else {
                    this->_mainLoopHeadless();
                }
            }
        } else {
            // Let's get this going
            if (!IsHeadlessMode()) {
                this->_mainLoop();
            } else {
                this->_mainLoopHeadless();
            }
        }
        
        if (!IsHeadlessMode()) {
            Engine::DisableGLContext();
        
            this->_shutdownOpenGL();
        }
        
        delete this->_scripting;
        
        Addon::Shutdown();
        
		Filesystem::Destroy();
        
		return 0;
    }
	
	int Application::Start(int argc, char const *argv[]) {
        // At this point Logger is not avalible
        Logger::Init();
        
        Platform::IsMainThread(); // Make sure we know which is the main thread
        
        Config::SetBoolean("core.log.enableConsole", true); // make sure we can log to the console right from the start
        
        Platform::SetRawCommandLine(argc, argv);
        
        if (!this->_parseCommandLine(argc, argv)) {
            return 1;
        }
        
        // The applicaiton has now parsed the command line
        
        Config::SetBoolean("core.log.enableConsole", false);
        
        this->_loadBasicConfigs();
        
        // Run _postStart
        if (Config::GetBoolean("core.catchErrors")) {
            try {
                return this->_postStart();
            } catch (...) {
                Platform::ShowMessageBox("Engine2D", "Engine2D has crashed and will now close", true);
                return 1;
            }
        } else {
            return this->_postStart();
        }
    }
    
    ApplicationPtr _singilton = NULL;
    
    ApplicationPtr GetAppSingilton() {
        if (_singilton == NULL) {
            _singilton = new Application();
        }
        return _singilton;
    }
	
} // namespace Engine

#ifdef _PLATFORM_WIN32
extern "C" _declspec(dllexport)
#else
extern "C"
#endif
int EngineMain(int argc, char const *argv[]) {
    return Engine::GetAppSingilton()->Start(argc, argv);
}
