/**
 * @file test_handlers.cpp
 * 
 * @author CJO
 * @brief Test suite responsible for testing the functionality of BatteryMessageHandlerImpl when processing Protobuf messages.
 */


#include <zephyr/ztest.h>

#include "logger_mock.h"
#include "usb_hid_mock.h"
#include "message_handler.h"
#include "message_dispatcher.h"
#include "battery_mock.h"
#include "battery_manager.h"
#include "message_manager.h"
#include "battery_message_handler_impl.h"
#include "battery_message_encoder.h"
#include "util.h"


K_MSGQ_DEFINE(msg_mock_queue, sizeof(MessageBuffer), 10, 1);

struct battery_message_handler_suite_fixture {
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	std::shared_ptr<LoggerMock> logger;
	std::unique_ptr<UsbHidMock> usb_mock;
	std::unique_ptr<Battery> test_battery_mock;
	std::unique_ptr<BatteryManager> bat_mngr;
	std::unique_ptr<MessageManager> msg_mngr;
	std::unique_ptr<BatteryMessageHandlerImpl> bat_msg_handler;
};

static void *battery_message_handler_suite_setup(void)
{
	struct battery_message_handler_suite_fixture *fixture = (struct battery_message_handler_suite_fixture*)k_malloc(sizeof(struct battery_message_handler_suite_fixture));
	zassume_not_null(fixture, NULL);
	fixture->logger = std::make_shared<LoggerMock>("");
	fixture->usb_mock = std::make_unique<UsbHidMock>(*fixture->logger);
  	return fixture;
}

static void battery_message_handler_suite_before(void* f)
{
	// Instantiate some fixture objects for general purpose use.
	struct battery_message_handler_suite_fixture *fixture = (struct battery_message_handler_suite_fixture *)f;
  	zassume_not_null(fixture, NULL);

	fixture->test_battery_mock.reset();
	fixture->bat_mngr.reset();
	fixture->msg_mngr.reset();
	fixture->bat_msg_handler.reset();
	fixture->test_battery_mock = std::make_unique<BatteryMock>();
	fixture->bat_mngr = std::make_unique<BatteryManager>(fixture->logger, std::move(fixture->test_battery_mock));
	fixture->msg_mngr = std::make_unique<MessageManager>(fixture->logger.get(), fixture->usb_mock.get(), &fixture->msg_proto, &fixture->dispatcher, &msg_mock_queue);
	fixture->bat_msg_handler = std::make_unique<BatteryMessageHandlerImpl>(*fixture->logger, *fixture->bat_mngr, *fixture->msg_mngr);
	fixture->dispatcher.AddHandler(*fixture->bat_msg_handler);
}

static void battery_message_handler_suite_teardown(void *f)
{
  k_free(f);
}

ZTEST_SUITE(battery_message_handler_suite, NULL, battery_message_handler_suite_setup, battery_message_handler_suite_before, NULL, battery_message_handler_suite_teardown);


/// RequestBatteryInfo ///

/// @brief Test message handling of RequestBatteryInfo.
/// Test that:
/// 1. Message can be decoded.
/// 2. A response is encoded.
/// 3. Handling involves triggering and getting battery sample. Response should contain sample data.
ZTEST_F(battery_message_handler_suite, test_handler_request_battery_info)
{
	// Setup some dummy variables and assign them to the mocked battery
    BatteryData test_data_exp;
    test_data_exp.temp = 20.0;
    test_data_exp.volt = 16.0;
    test_data_exp.current = 5.5;
    test_data_exp.remaining_capacity = 5000;
    test_data_exp.status = 0xff;
    test_data_exp.relative_charge_state = 90;
    test_data_exp.cycle_count = 120;

    LoggerMock logger("");
    std::unique_ptr<BatteryMock> test_mock = std::make_unique<BatteryMock>();
    test_mock->SetTestTemp(test_data_exp.temp);
    test_mock->SetTestVolt(test_data_exp.volt);
    test_mock->SetTestCurrent(test_data_exp.current);
    test_mock->SetTestRemCap(test_data_exp.remaining_capacity);
    test_mock->SetTestStatus(test_data_exp.status);
    test_mock->SetTestRelChargeState(test_data_exp.relative_charge_state);
    test_mock->SetTestCycleCount(test_data_exp.cycle_count);

    std::unique_ptr<Battery> test_battery(std::move(test_mock));
	fixture->bat_mngr.reset();
    fixture->bat_mngr = std::make_unique<BatteryManager>(fixture->logger, std::move(test_battery));
	
	// Encode a RequestBatteryInfo message.
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeRequestBatteryInfo(message), "Could not encode RequestBatteryInfo message.");
	zassert_equal(message.msg_type, INCOMING);

	// 1. Assert decodable
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryInfo Outer message.");
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle RequestBatteryInfo message.");
	
	// 2. & 3. Check encoded Response message is as expected.
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryInfo Outer message");
	BatteryInfo bi = BatteryInfo_init_zero;
	zassert_true(fixture->msg_proto.DecodeInnerMessage(BatteryInfo_fields, &bi), "Could not decode BatteryInfo message");
	zassert_true(bi.valid);
	zassert_equal(bi.temperature, test_data_exp.temp);
	zassert_equal(bi.voltage, test_data_exp.volt);
	zassert_equal(bi.current, test_data_exp.current);
	zassert_equal(bi.rem_capacity, test_data_exp.remaining_capacity);
	zassert_equal(bi.status, test_data_exp.status);
	zassert_equal(bi.relative_state_of_charge, test_data_exp.relative_charge_state);
	zassert_equal(bi.cycle_count, test_data_exp.cycle_count);
}


/// BatteryInfo ///

ZTEST_F(battery_message_handler_suite, test_handler_battery_info)
{
	// Encode a BatteryInfo message
	MessageBuffer message;
	BatteryData test_data_exp;
    test_data_exp.temp = 20.0;
    test_data_exp.volt = 16.0;
    test_data_exp.current = 5.5;
    test_data_exp.remaining_capacity = 5000;
    test_data_exp.status = 0xff;
    test_data_exp.relative_charge_state = 90;
    test_data_exp.cycle_count = 120;
	zassert_true(BatteryMessageEncoder::EncodeBatteryInfo(message, true, test_data_exp.temp, test_data_exp.volt, test_data_exp.current, 
																   test_data_exp.remaining_capacity, test_data_exp.status, 
																   test_data_exp.relative_charge_state, test_data_exp.cycle_count), "Could not encode BatteryInfo message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryInfo Outer message.");
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle BatteryInfo message.");
	// No encoded response
	zassert_equal(message.msg_type, INCOMING);
}


/// RequestBatteryNotifications ///

/// @brief Test that an enable==true request adds a subscriber and encodes a response.
ZTEST_F(battery_message_handler_suite, test_handle_true_subcribe_request)
{
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeRequestBatteryNotifications(message, true), "Could not encode RequestBatteryNotifications message.");
	zassert_equal(message.msg_type, msg_type_t::INCOMING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle RequestBatteryNotifications message.");
	zassert_equal(fixture->bat_mngr->GetSubscriberCount(), 1);
	zassert_true(fixture->bat_mngr->GetCpuSubscribed());

	// Check encoded Response message is as expected
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	ResponseBatteryNotifications req_bn = ResponseBatteryNotifications_init_zero;
	zassert_true(fixture->msg_proto.DecodeInnerMessage(ResponseBatteryNotifications_fields, &req_bn), "Could not decode ResponseBatteryNotifications message");
}

/// @brief Test that an enable==false does not add a subscriber (and currently no response).
ZTEST_F(battery_message_handler_suite, test_handle_false_subcribe_request)
{
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeRequestBatteryNotifications(message, false), "Could not encode RequestBatteryNotifications message.");
	zassert_equal(message.msg_type, msg_type_t::INCOMING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle RequestBatteryNotifications message.");

	// Check that no acknowledgement leads to message buffer being unmodified.
	zassert_equal(message.msg_type, msg_type_t::INCOMING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	RequestBatteryNotifications res_bn = RequestBatteryNotifications_init_zero;
	zassert_true(fixture->msg_proto.DecodeInnerMessage(RequestBatteryNotifications_fields, &res_bn), "Expected RequestBatteryNotifications message, but couldn't decode");
}

/// @brief Test that multiple subscribe requests from CPU does not lead to multiple subscribe callbacks being registered.
ZTEST_F(battery_message_handler_suite, test_handle_multiple_subscribe_request)
{	
	for(int i = 0; i < 3; i++)
	{
		MessageBuffer message;
		zassert_true(BatteryMessageEncoder::EncodeRequestBatteryNotifications(message, true), "Could not encode RequestBatteryNotifications message.");
		zassert_equal(message.msg_type, msg_type_t::INCOMING);
		zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
		zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle RequestBatteryNotifications message.");
		
		// Check encoded Response message is as expected
		zassert_equal(message.msg_type, msg_type_t::OUTGOING);
		zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
		ResponseBatteryNotifications req_bn = ResponseBatteryNotifications_init_zero;
		zassert_true(fixture->msg_proto.DecodeInnerMessage(ResponseBatteryNotifications_fields, &req_bn), "Could not decode ResponseBatteryNotifications message");
	}
	zassert_equal(fixture->bat_mngr->GetSubscriberCount(), 1);
	zassert_true(fixture->bat_mngr->GetCpuSubscribed());
}

ZTEST_F(battery_message_handler_suite, test_handle_resubscribe_request)
{	
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeRequestBatteryNotifications(message, true), "Could not encode RequestBatteryNotifications message.");
	zassert_equal(message.msg_type, msg_type_t::INCOMING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle RequestBatteryNotifications message.");
	zassert_equal(fixture->bat_mngr->GetSubscriberCount(), 1);
	zassert_true(fixture->bat_mngr->GetCpuSubscribed());
	// Check encoded Response message is as expected
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	ResponseBatteryNotifications req_bn = ResponseBatteryNotifications_init_zero;
	zassert_true(fixture->msg_proto.DecodeInnerMessage(ResponseBatteryNotifications_fields, &req_bn), "Could not decode ResponseBatteryNotifications message");

	fixture->bat_mngr->ClearSubscribers();
	zassert_equal(fixture->bat_mngr->GetSubscriberCount(), 0);
	zassert_false(fixture->bat_mngr->GetCpuSubscribed());
	zassert_true(BatteryMessageEncoder::EncodeRequestBatteryNotifications(message, true), "Could not encode RequestBatteryNotifications message.");
	message.msg_type = INCOMING;
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle RequestBatteryNotifications message.");
	zassert_equal(fixture->bat_mngr->GetSubscriberCount(), 1);
	zassert_true(fixture->bat_mngr->GetCpuSubscribed());
	// Check encoded Response message is as expected
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	req_bn = ResponseBatteryNotifications_init_zero;
	zassert_true(fixture->msg_proto.DecodeInnerMessage(ResponseBatteryNotifications_fields, &req_bn), "Could not decode ResponseBatteryNotifications message");
}


/// ResponseBatteryNotifications ///

ZTEST_F(battery_message_handler_suite, test_handler_subcribe_response)
{
	// Encode a response BatteryNotifications message
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeResponseBatteryNotifications(message), "Could not encode ResponseBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle ResponseBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
}