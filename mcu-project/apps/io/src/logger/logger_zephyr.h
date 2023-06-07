#pragma once

#include "logger.h"

class LoggerZephyr : public Logger {
public:
    LoggerZephyr(const char* name) : name_(name){};
    virtual ~LoggerZephyr() = default;
    virtual void inf(const char* msg) override;
    virtual void wrn(const char* msg) override;
    virtual void err(const char* msg) override;
    virtual void dbg(const char* msg) override;

private:
    const char* name_;
};