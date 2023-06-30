#include "inclinometer_message_handler_impl.h"

LOG_MODULE_REGISTER(incl_msg_hdlr, CONFIG_INCLINOMETER_MSG_HDLR_LOG_LEVEL);

InclinometerMessageHandlerImpl::InclinometerMessageHandlerImpl(InclinometerManager& incl)
    : incl_{incl}
{}

/// @brief Message handler for SetSamplePeriod. Set sample period to the contained number. Period
/// should be a positive integer.
/// @param msg   Message handler default.
/// @param buffer   Message handler default.
/// @return True for success, false otherwise.
bool InclinometerMessageHandlerImpl::HandleSetSamplePeriod(MessageProto& msg, MessageBuffer& buffer)
{
    SetSamplePeriod sr = SetSamplePeriod_init_zero;
    if (!msg.DecodeInnerMessage(SetSamplePeriod_fields, &sr)) {
        return false;
    }

    incl_.ChangeTimer(sr.period);

    if (!InclinometerMessageEncoder::EncodeRespSamplePeriod(buffer))  // Encode response
    {
        LOG_WRN("Failed to Encode RespSamplePeriod");
        return false;
    }

    buffer.msg_type = OUTGOING;
    return true;
}

bool InclinometerMessageHandlerImpl::HandleRespSamplePeriod(MessageProto& msg,
                                                            MessageBuffer& buffer)
{
    RespSamplePeriod sr = RespSamplePeriod_init_zero;
    if (!msg.DecodeInnerMessage(SetSamplePeriod_fields, &sr)) {
        return false;
    }

    return true;
}

/// @brief
/// @param msg
/// @param buffer
/// @return
bool InclinometerMessageHandlerImpl::HandleGetSamplePeriod(MessageProto& msg, MessageBuffer& buffer)
{
    GetSamplePeriod sr = GetSamplePeriod_init_zero;
    if (!msg.DecodeInnerMessage(GetSamplePeriod_fields, &sr)) {
        return false;
    }

    uint32_t sample_period = incl_.GetSamplePeriod();

    if (!InclinometerMessageEncoder::EncodeRespGetSamplePeriod(buffer,
                                                               sample_period))  // Encode response
    {
        LOG_WRN("Failed to Encode RespSamplePeriod");
        return false;
    }

    buffer.msg_type = OUTGOING;
    return true;
}

bool InclinometerMessageHandlerImpl::HandleRespGetSamplePeriod(MessageProto& msg,
                                                               MessageBuffer& buffer)
{
    RespGetSamplePeriod sr = RespGetSamplePeriod_init_zero;
    if (!msg.DecodeInnerMessage(SetSamplePeriod_fields, &sr)) {
        return false;
    }

    return true;
}

/// @brief Message handler for "GetAngle" messages
///     Triggers a return message containing the X, Y, and Z angle measurements.
/// @param msg   Message handler default
/// @param buffer    Message handler default
/// @return  True for success, false otherwise.
bool InclinometerMessageHandlerImpl::HandleGetAngle(MessageProto& msg, MessageBuffer& buffer)
{
    GetAngle sr = GetAngle_init_zero;
    if (!msg.DecodeInnerMessage(GetAngle_fields, &sr)) {
        return false;
    }

    // Handle message
    float x_angle = incl_.GetLastXAngle();
    float y_angle = incl_.GetLastYAngle();
    float z_angle = incl_.GetLastZAngle();

    // Handle return message
    if (!InclinometerMessageEncoder::EncodeXYZAngle(buffer, x_angle, y_angle,
                                                    z_angle))  // Encode response
    {
        LOG_WRN("Failed to Encode RespGetAngle");
        return false;
    }

    buffer.msg_type = OUTGOING;
    return true;
}

bool InclinometerMessageHandlerImpl::HandleXYZAngle(MessageProto& msg, MessageBuffer& buffer)
{
    XYZAngle sr = XYZAngle_init_zero;
    if (!msg.DecodeInnerMessage(XYZAngle_fields, &sr)) {
        return false;
    }

    return true;
}