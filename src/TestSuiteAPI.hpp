#pragma once

#include <string>

namespace Engine {
    class Test {
    public:
        virtual void Setup() {}
        virtual void Run() {}
        virtual void PullDown() {}
        
        virtual std::string GetName() { return "Invalid Test"; }
        
        bool GetFailed();
        
    protected:
        void Assert(std::string name, bool value);
        
        void FailTest();
        
    private:
        bool _failed;
    };
}