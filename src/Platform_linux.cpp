/*
   Filename: Platform_mac.cpp
   Purpose:  Platform specfic interfaces

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

#include "Platform.hpp"

#include "Logger.hpp"

#include <iostream>
#include <cstdio>

#include <fcntl.h>

#include <pwd.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include <execinfo.h>
#include <cxxabi.h>

#include <dlfcn.h>

#include <pthread.h>

#include <uuid/uuid.h>

namespace Engine {
    namespace Platform {
        
        ENGINE_CLASS(LinuxLibary);
        
        class LinuxLibary : public Libary {
        public:
            LinuxLibary() {
                // This should get a pointer for the current process, not 100% effective
                // but it will work fine on OSX
                this->modulePointer = dlopen(GetRawCommandLineArgV()[0], RTLD_LAZY);
            }
            
            LinuxLibary(std::string modulePath) {
                this->modulePointer = dlopen(modulePath.c_str(), RTLD_LAZY);
                if (this->modulePointer == NULL) {
                    Logger::begin("OSXLibrary", Logger::LogLevel_Error) << "Error loading: " << modulePointer << " : " << dlerror() << Logger::end();
                }
            }
            
            ~LinuxLibary() override {
                dlclose(this->modulePointer);
                this->modulePointer = NULL;
            }
            
            bool IsValid() override {
                return this->modulePointer != NULL;
            }
            
            ENGINE_FARPROC GetMethod(std::string name) override {
                return (ENGINE_FARPROC) dlsym(this->modulePointer, name.c_str());
            }
            
            bool CallMethod(std::string name) override {
                ENGINE_FARPROC m = this->GetMethod(name);
                if (m != NULL) {
                    m();
                    return 0;
                } else {
                    return 1;
                }
            }
            
            std::vector<std::string> GetExportedMethods() override {
                std::vector<std::string> ret;
                
                // TODO: open the module file using Filesystem and load the content
                //          this is going to be a real pain
                
                return ret;
            }
            
        private:
            void* modulePointer;
        };
        
        ENGINE_CLASS(LinuxThread);
        
        class LinuxThread : public Thread {
        public:
            LinuxThread(ThreadMethod entry, void* args) {
                if (pthread_create(&this->_thread, NULL, entry, args)) {
                    std::cout << "ERROR: pthread_create failed" << std::endl;
                }
                
                this->_uuid = GenerateUUID();
            }
            
            LinuxThread(pthread_t thread) {
                _thread = thread;
            }
            
            void Terminate() override {
                
            }
            
            void Exit(void* ret) override {
                pthread_exit(ret);
            }
            
            UUID GetThreadID() override {
                return this->_uuid;
            }
            
        private:
            pthread_t _thread;
            UUID _uuid;
        };
        
        ENGINE_CLASS(LinuxMutex);
        
        class LinuxMutex : public Mutex {
        public:
            LinuxMutex() {
                pthread_mutex_init(&this->_mutex, NULL);
            }
            
            ~LinuxMutex() override {
                pthread_mutex_destroy(&this->_mutex);
            }
            
            bool SafeEnter() override {
                if (!this->_entered) {
                    this->Enter();
                    return true;
                } else {
                    return false;
                }
            }
            
            void Enter() override {
                pthread_mutex_lock(&this->_mutex);
                this->_entered = true;
            }
            
            void Exit() override {
                pthread_mutex_unlock(&this->_mutex);
                this->_entered = false;
            }
        private:
            pthread_mutex_t _mutex;
            bool _entered = false;
        };
        
        class LinuxMemoryMappedFile : public MemoryMappedFile {
        public:
            LinuxMemoryMappedFile(int fd) : _fd(fd) { }
            
            MemoryMappedRegionPtr MapRegion(unsigned long offset, size_t size) override {
                assert(offset % 4096 == 0);
                // Make sure the file is of the right length
                size_t fileSize = 0;
                struct stat st;
                if (fstat(this->_fd, &st) == 0) {
                    fileSize = st.st_size;
                }
                if (offset + size > fileSize) {
                    lseek(this->_fd, (offset + size) - 1, SEEK_SET);
                    write(this->_fd, "", 1);
                }
                
                void* region = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, this->_fd, offset);
                if (errno == EINVAL) { throw errno; }
                this->_mappedRegions++;
                return new MemoryMappedRegion(this, region, size);
            }
            
            void UnmapRegion(MemoryMappedRegionPtr r) override {
                munmap(r->_data, r->_length);
                delete r;
                this->_mappedRegions--;
            }
            
            bool IsValid() override {
                return fcntl(this->_fd, F_GETFD) != -1 || errno != EBADF;
            }
            
            void Close() override {
                assert(this->_mappedRegions == 0);
                close(this->_fd);
            }
            
        private:
            int _fd;
            size_t _mappedRegions = 0;
        };
        
        int _argc;
        const char** _argv;
        
        void SetRawCommandLine(int argc, const char** argv) {
            _argc = argc;
            _argv = argv;
        }
        
        int GetRawCommandLineArgC() {
            return _argc;
        }
        
        const char** GetRawCommandLineArgV() {
            return _argv;
        }
        
        std::string GetExecutablePath() {
            return "";
        }
        
        engine_memory_info GetMemoryInfo() {
            engine_memory_info ret;
            
            return ret;
        }
        
        double GetMemoryUsage() {
            engine_memory_info mem = GetMemoryInfo();
            return (double) mem.myPhysicalUsed / (double) mem.totalPhysical;
        }
        
        int GetProcesserCount() {
            return (int) sysconf(_SC_NPROCESSORS_ONLN); // Show me a system where that
                // function will return more the 2^32 processers
        }
        
        LibaryPtr OpenLibary(std::string path) {
            return new LinuxLibary(path);
        }
        
        LibaryPtr GetThisLibary() {
            return new LinuxLibary();
        }
        
        ThreadPtr CreateThread(ThreadMethod entry, void* threadArgs) {
            return new LinuxThread(entry, threadArgs);
        }
        
        ThreadPtr GetCurrentThread() {
            return new LinuxThread(pthread_self());
        }

        bool IsMainThread() {
            static int mainThreadID = 0;
            if (mainThreadID == 0) {
                mainThreadID = syscall(SYS_gettid);
            }
            return mainThreadID == syscall(SYS_gettid);
        }
        
        MutexPtr CreateMutex() {
            return new LinuxMutex();
        }
        
        MemoryMappedFilePtr OpenMemoryMappedFile(std::string filename, FileMode mode) {
            int fmode;
            switch (mode) {
                case FileMode::Read:    fmode = O_RDONLY; break;
                case FileMode::Write:   fmode = O_RDWR;   break;
            }
            int fd = open(filename.c_str(), fmode);
            assert(fd != -1);
            return new LinuxMemoryMappedFile(fd);
        }
        
        UUID GenerateUUID() {
            UUID uuid;
            uuid_generate((unsigned char*) &uuid);
            return uuid;
        }
        
        std::string StringifyUUID(UUID uuidArr) {
            char* uuidString = new char[37];
            uuid_unparse((unsigned char*) &uuidArr, uuidString);
            std::string ret = std::string(uuidString);
            delete [] uuidString;
            return ret;
        }
        
        UUID ParseUUID(std::string uuidStr) {
            UUID uuidArray;
            uuid_parse(uuidStr.c_str(), (unsigned char*) &uuidArray);
            return uuidArray;
        }
        
        double GetTime() {
            static double _startTime = -1;
            
            timeval _time;
            gettimeofday(&_time, NULL);
            
            double time = _time.tv_sec + (_time.tv_usec * 0.000001);
            
            if (_startTime < 0) {
                _startTime = time;
                return 0;
            } else {
                return time - _startTime;
            }
        }
        
        bool ShowMessageBox(std::string title, std::string text, bool modal) {
            
            return true;
        }
        
        std::string GetUsername() {
            struct passwd *userInfo = getpwuid(getuid());
            return std::string(userInfo->pw_name);
        }
        
        void DumpStackTrace() {
            void* callstack[128];
            int frames = backtrace(callstack, 128);
            for (int i = 1; i < frames; i++) {
                Dl_info info;
                if (dladdr(callstack[i], &info) > 0) {
                    char* realname;
                    int status = 0;
                    realname = abi::__cxa_demangle(info.dli_sname, 0, 0, &status);
                    if (status == 0) {
                        printf("  %i | 0x%016lx | %-50s | %s\n", i, (uint64_t) callstack[i], info.dli_fname, realname);
                    } else {
                        printf("  %i | 0x%016lx | %-50s | %s\n", i, (uint64_t) callstack[i], info.dli_fname, info.dli_sname);
                    }
                    free(realname);
                } else {
                    printf("  %i | 0x%016lx | [unknown symbol]\n", i, (uint64_t) callstack[i]);
                }
            }
        }
        
        int ShellExecute(std::string path) {
            return system(("open " + path).c_str());
        }
        
        void Sleep(int timeS) {
            sleep(timeS);
        }
        
        void NanoSleep(int timeNS) {
            usleep(timeNS);
        }
        
        long CryptBytes(unsigned char* buffer, long count) {
            FILE *fp = std::fopen("/dev/urandom", "r");
            
            if (!fp) {
                return -1;
            }
            
            long ret = fread(buffer, sizeof(unsigned char), count, fp);
            
            fclose(fp);
            
            return ret;
        }
    }
}
