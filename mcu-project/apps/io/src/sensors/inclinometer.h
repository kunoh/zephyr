#pragma once

#include "util.h"

class Inclinometer {
public:
    ///
    /// @brief Destroy the Inclinometer object
    ///
    virtual ~Inclinometer() = default;

    ///
    /// @brief Checks Device is ready.
    /// @return 0: SUCCESS Device is ready to use, 1: ERROR Device reports not ready, or is missing.
    ///
    virtual int Init() = 0;

    ///
    /// @brief TBI! Write settings to inclinometer TBI!
    /// @return  TRUE
    ///
    virtual bool Write() = 0;

    ///
    /// @brief Reads data from inclinometer
    /// Triggers a fetch of samples from sensor, and transfers them from the driver to private
    /// variables in this class.
    /// @return status code
    ///
    virtual bool Read() = 0;

    ///
    /// @brief Get-function to retrieve angle values.
    /// @param XYZAngle Array of MINIMUM 3 doubles. MIN LENGTH 3!
    /// Returns the last sampled X, Y, and Z angle data into the passed array.
    /// Unit: degrees.
    ///
    virtual void GetAngle(SensorSampleData *XYZAngle) = 0;
};