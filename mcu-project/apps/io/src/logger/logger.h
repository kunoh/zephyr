#pragma once

class Logger {
public:
    virtual ~Logger() = default;
    virtual void inf(const char* msg) = 0;
    virtual void wrn(const char* msg) = 0;
    virtual void err(const char* msg) = 0;
    virtual void dbg(const char* msg) = 0;
};