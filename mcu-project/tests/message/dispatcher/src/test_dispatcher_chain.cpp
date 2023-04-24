#include <zephyr/ztest.h>

#include "message_dispatcher.h"

#include "system_message_handler_mock.h"
#include "display_message_handler_mock.h"
#include "inclinometer_message_handler_mock.h"

#include "system_message_encoder.h"
#include "display_message_encoder.h"
#include "inclinometer_message_encoder.h"

#include "display_mock.h"
#include "inclinometer_mock.h"

#include "logger_mock.h"

ZTEST_SUITE(system_message_dispatching_suite, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(display_message_dispatching_suite, NULL, NULL, NULL, NULL, NULL);

ZTEST(system_message_dispatching_suite, test_message_version)
{
	LoggerMock logger("");
	MessageProto msg_proto;

	// Initialize message dispatcher and add handler
	SystemMessageHandlerMock sys_mock(logger);
	MessageDispatcher dispatcher;
	dispatcher.AddHandler(sys_mock);

	// Encode a request version message
	MessageBuffer message;
	zassert_true(SystemMessageEncoder::EncodeRequestVersion(message), "Could not encode RequestVersion message");

	// Decoder the outer message and handle the inner message which will generate a response
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode Version Outer message");
	zassert_true(dispatcher.Handle(msg_proto, message), "Could not handle RequestVersion Message");

	// Check encoded Response message is as expected
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode Version Outer message");
	ResponseVersion rv = ResponseVersion_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(ResponseVersion_fields, &rv), "Could not decode ResponseVersion message");
	zassert_equal(2, rv.version, "Got a wrong version number, %d", rv.version);
};

ZTEST(display_message_dispatching_suite, test_message_stop_spinner)
{
	LoggerMock logger("");
	MessageProto msg_proto;

	// Initialize message dispatcher and add handler
	DisplayMock disp(logger);
	DisplayMessageHandlerMock disp_mock(logger, disp);
	MessageDispatcher dispatcher;
	dispatcher.AddHandler(disp_mock);

	// Encode a request StopSpinner message
	MessageBuffer message;
	zassert_true(DisplayMessageEncoder::EncodeRequestStopSpinner(message), "Could not encode RequestStopSpinner message");

	// Use message dispatcher to handle message
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode StopSpinner Outer message");
	zassert_true(dispatcher.Handle(msg_proto, message), "Could not handle RequestStopSpinner Message");

	// Check encoded Response message is as expected
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode StopSpinner Outer message");
	ResponseStopSpinner rv = ResponseStopSpinner_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(ResponseStopSpinner_fields, &rv), "Could not decode ResponseStopSpinner message");
	zassert_true(rv.status, "Got a wrong status, %d", rv.status);
};

ZTEST(display_message_dispatching_suite, test_message_new_frame)
{
	LoggerMock logger("");
	MessageProto msg_proto;

	// Initialize message dispatcher and add handler
	DisplayMock disp(logger);
	DisplayMessageHandlerMock disp_mock(logger, disp);
	MessageDispatcher dispatcher;
	dispatcher.AddHandler(disp_mock);

	// Encode a request NewFrame message
	MessageBuffer message;
	zassert_true(DisplayMessageEncoder::EncodeRequestNewFrame(message), "Could not encode RequestNewFrame message");

	// Use message dispatcher to handle message
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode NewFrame Outer message");
	zassert_true(dispatcher.Handle(msg_proto, message), "Could not handle RequestNewFrame Message");

	// Check encoded Response message is as expected
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode NewFrame Outer message");
	ResponseNewFrame rv = ResponseNewFrame_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(ResponseNewFrame_fields, &rv), "Could not decode ResponseNewFrame message");
	zassert_true(rv.status, "Got a wrong status, %d", rv.status);
};