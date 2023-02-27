#pragma once
#include <inclinometer.h>

class InclinometerImpl : public Inclinometer {
public:
    virtual ~InclinometerImpl() = default;
    bool write() override;
};