#pragma once

#include "logger.h"

class LoggerMock : public Logger {
public:
    LoggerMock(const char* name) : name_(name){};
    virtual ~LoggerMock() = default;
    virtual void inf(const char* msg) override;
    virtual void wrn(const char* msg) override;
    virtual void err(const char* msg) override;
    virtual void dbg(const char* msg) override;

private:
    const char* name_;
};