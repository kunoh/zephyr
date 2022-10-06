#include <ztest.h>
#include "message_encoder.hpp"
#include "message_handler.hpp"

extern bool DecodeInnerMessage(Message& msg, const pb_msgdesc_t *fields, void *dest_struct);

void test_message_version(void)
{
	/* Encode a request version message */
	MessageBuffer message;
	zassert_true(MessageEncoder::EncodeRequestVersion(message), "Could not encode Request message");
	
	/* Use message dispatcher to handle message */
	Message msg;
	MessageDispatcher dispatcher;
    dispatcher.Init();

	zassert_true(msg.InitializeMessageOuter(message.buffer, message.message_length), "Could not decode Outer message");
	zassert_true(dispatcher.Handle(message, msg), "Could not handle Request Message");
	
	/* Check encoded Response message is as expected */
	zassert_true(msg.InitializeMessageOuter(message.buffer, message.message_length), "Could not decode Outer message");
	ResponseVersion rv = ResponseVersion_init_zero;
	zassert_true(DecodeInnerMessage(msg, ResponseVersion_fields, &rv), "Could not decode Response message");
	zassert_equal(2, rv.version, "Got a wrong version number, %d", rv.version);
}

void test_main(void)
{
	ztest_test_suite(message_version_unit_tests,
			 ztest_unit_test(test_message_version)
	);
	ztest_run_test_suite(message_version_unit_tests);
}