#include <ztest.h>
#include "message_encoder.h"
#include "message_handler.h"

extern bool DecodeInnerMessage(Message& msg, const pb_msgdesc_t *fields, void *dest_struct);

//Display abstractions
void display_stop_spinner(void) {};
void display_signal_new_frame(void) {};

void test_message_version(void)
{
	/* Encode a request version message */
	MessageBuffer message;
	zassert_true(MessageEncoder::EncodeRequestVersion(message), "Could not encode RequestVersion message");
	
	/* Use message dispatcher to handle message */
	Message msg;
	MessageDispatcher dispatcher;
    dispatcher.Init();

	zassert_true(msg.InitializeMessageOuter(message.buffer, message.message_length), "Could not decode Version Outer message");
	zassert_true(dispatcher.Handle(message, msg), "Could not handle RequestVersion Message");
	
	/* Check encoded Response message is as expected */
	zassert_true(msg.InitializeMessageOuter(message.buffer, message.message_length), "Could not decode Version Outer message");
	ResponseVersion rv = ResponseVersion_init_zero;
	zassert_true(DecodeInnerMessage(msg, ResponseVersion_fields, &rv), "Could not decode ResponseVersion message");
	zassert_equal(2, rv.version, "Got a wrong version number, %d", rv.version);
}

void test_message_stop_spinner(void)
{
	/* Encode a request StopSpinner message */
	MessageBuffer message;
	zassert_true(MessageEncoder::EncodeRequestStopSpinner(message), "Could not encode RequestStopSpinner message");
	
	/* Use message dispatcher to handle message */
	Message msg;
	MessageDispatcher dispatcher;
    dispatcher.Init();

	zassert_true(msg.InitializeMessageOuter(message.buffer, message.message_length), "Could not decode StopSpinner Outer message");
	zassert_true(dispatcher.Handle(message, msg), "Could not handle RequestStopSpinner Message");
	
	/* Check encoded Response message is as expected */
	zassert_true(msg.InitializeMessageOuter(message.buffer, message.message_length), "Could not decode StopSpinner Outer message");
	ResponseStopSpinner rv = ResponseStopSpinner_init_zero;
	zassert_true(DecodeInnerMessage(msg, ResponseStopSpinner_fields, &rv), "Could not decode ResponseStopSpinner message");
	zassert_true(rv.status, "Got a wrong status, %d", rv.status);
}

void test_message_new_frame(void)
{
	/* Encode a request NewFrame message */
	MessageBuffer message;
	zassert_true(MessageEncoder::EncodeRequestNewFrame(message), "Could not encode RequestNewFrame message");
	
	/* Use message dispatcher to handle message */
	Message msg;
	MessageDispatcher dispatcher;
    dispatcher.Init();

	zassert_true(msg.InitializeMessageOuter(message.buffer, message.message_length), "Could not decode NewFrame Outer message");
	zassert_true(dispatcher.Handle(message, msg), "Could not handle RequestNewFrame Message");
	
	/* Check encoded Response message is as expected */
	zassert_true(msg.InitializeMessageOuter(message.buffer, message.message_length), "Could not decode NewFrame Outer message");
	ResponseNewFrame rv = ResponseNewFrame_init_zero;
	zassert_true(DecodeInnerMessage(msg, ResponseNewFrame_fields, &rv), "Could not decode ResponseNewFrame message");
	zassert_true(rv.status, "Got a wrong status, %d", rv.status);
}

void test_main(void)
{
	ztest_test_suite(message_version_unit_tests,
			 ztest_unit_test(test_message_version),
			 ztest_unit_test(test_message_stop_spinner),
			 ztest_unit_test(test_message_new_frame)
	);
	ztest_run_test_suite(message_version_unit_tests);
}