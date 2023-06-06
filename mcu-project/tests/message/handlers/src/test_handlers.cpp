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
#include "battery_charger_mock.h"
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
	std::unique_ptr<BatteryCharger> test_battery_charger_mock;
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
	fixture->test_battery_charger_mock.reset();
	fixture->bat_mngr.reset();
	fixture->msg_mngr.reset();
	fixture->bat_msg_handler.reset();
	fixture->test_battery_mock = std::make_unique<BatteryMock>();
	fixture->test_battery_charger_mock = std::make_unique<BatteryChargerMock>();
	fixture->bat_mngr = std::make_unique<BatteryManager>(fixture->logger, std::move(fixture->test_battery_mock), std::move(fixture->test_battery_charger_mock));
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
    BatteryGeneralData test_data_exp;
    test_data_exp.temp = 20.0;
    test_data_exp.volt = 16.0;
    test_data_exp.current = 5.5;
    test_data_exp.remaining_capacity = 5000;
    test_data_exp.relative_charge_state = 90;
    test_data_exp.cycle_count = 120;

    std::unique_ptr<BatteryMock> test_mock = std::make_unique<BatteryMock>();
    test_mock->SetTestTemp(test_data_exp.temp);
    test_mock->SetTestVolt(test_data_exp.volt);
    test_mock->SetTestCurrent(test_data_exp.current);
    test_mock->SetTestRemCap(test_data_exp.remaining_capacity);
    test_mock->SetTestRelChargeState(test_data_exp.relative_charge_state);
    test_mock->SetTestCycleCount(test_data_exp.cycle_count);

    std::unique_ptr<Battery> test_battery(std::move(test_mock));
	std::unique_ptr<BatteryChargerMock> test_charger_mock = std::make_unique<BatteryChargerMock>();
	fixture->bat_mngr.reset();
    fixture->bat_mngr = std::make_unique<BatteryManager>(fixture->logger, std::move(test_battery), std::move(test_charger_mock));

	// Encode a RequestBatteryInfo message.
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryGeneralInfo(message), "Could not encode ReqBatteryGeneralInfo message.");
	zassert_equal(message.msg_type, INCOMING);
	
	// 1. Assert decodable
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message.");
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle ReqBatteryGeneralInfo message.");
	
	// 2. & 3. Check encoded Response message is as expected.
	// When sampling hasn't been started we expect default invalid values.
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message");
	BatteryGeneralInfo bgi = BatteryGeneralInfo_init_zero;
	zassert_true(fixture->msg_proto.DecodeInnerMessage(BatteryGeneralInfo_fields, &bgi), "Could not decode BatteryGeneralInfo message");
	zassert_equal(bgi.temperature, DEFAULT_INVALID_BAT_FLOAT);
	zassert_equal(bgi.voltage, DEFAULT_INVALID_BAT_FLOAT);
	zassert_equal(bgi.current, DEFAULT_INVALID_BAT_FLOAT);
	zassert_equal(bgi.rem_capacity, DEFAULT_INVALID_BAT_INT);
	zassert_equal(bgi.relative_state_of_charge, DEFAULT_INVALID_BAT_INT);
	zassert_equal(bgi.cycle_count, DEFAULT_INVALID_BAT_INT);

	fixture->bat_mngr->StartSampling(GENERAL, 20, 60000);
	k_msleep(30);
	
	message.msg_type = INCOMING;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryGeneralInfo(message), "Could not encode ReqBatteryGeneralInfo message.");
	zassert_equal(message.msg_type, INCOMING);
	
	// 1. Assert decodable
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message.");
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle ReqBatteryGeneralInfo message.");

	// 2. & 3. Check encoded Response message is as expected.
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message");
	bgi = BatteryGeneralInfo_init_zero;
	zassert_true(fixture->msg_proto.DecodeInnerMessage(BatteryGeneralInfo_fields, &bgi), "Could not decode BatteryGeneralInfo message");
	
	zassert_equal(bgi.temperature, test_data_exp.temp);
	zassert_equal(bgi.voltage, test_data_exp.volt);
	zassert_equal(bgi.current, test_data_exp.current);
	zassert_equal(bgi.rem_capacity, test_data_exp.remaining_capacity);
	zassert_equal(bgi.relative_state_of_charge, test_data_exp.relative_charge_state);
	zassert_equal(bgi.cycle_count, test_data_exp.cycle_count);

	// If the sampling timer is stopped we expect request-responses with invalid data.
	fixture->bat_mngr->StopSampling(GENERAL);
	message.msg_type = INCOMING;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryGeneralInfo(message), "Could not encode ReqBatteryGeneralInfo message.");
	zassert_equal(message.msg_type, INCOMING);
	
	// 1. Assert decodable
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message.");
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle ReqBatteryGeneralInfo message.");

	// 2. & 3. Check encoded Response message is as expected.
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message");
	bgi = BatteryGeneralInfo_init_zero;
	zassert_true(fixture->msg_proto.DecodeInnerMessage(BatteryGeneralInfo_fields, &bgi), "Could not decode BatteryGeneralInfo message");
	
	zassert_equal(bgi.temperature, DEFAULT_INVALID_BAT_FLOAT);
	zassert_equal(bgi.voltage, DEFAULT_INVALID_BAT_FLOAT);
	zassert_equal(bgi.current, DEFAULT_INVALID_BAT_FLOAT);
	zassert_equal(bgi.rem_capacity, DEFAULT_INVALID_BAT_INT);
	zassert_equal(bgi.relative_state_of_charge, DEFAULT_INVALID_BAT_INT);
	zassert_equal(bgi.cycle_count, DEFAULT_INVALID_BAT_INT);
}


/// BatteryInfo ///

ZTEST_F(battery_message_handler_suite, test_handler_battery_info)
{
	// Encode a BatteryInfo message
	MessageBuffer message;
	BatteryGeneralData test_data_exp;
    test_data_exp.temp = 20.0;
    test_data_exp.volt = 16.0;
    test_data_exp.current = 5.5;
    test_data_exp.remaining_capacity = 5000;
    test_data_exp.relative_charge_state = 90;
    test_data_exp.cycle_count = 120;
	zassert_true(BatteryMessageEncoder::EncodeBatteryGeneralInfo(message, test_data_exp.temp, test_data_exp.volt, test_data_exp.current, 
																   test_data_exp.remaining_capacity, test_data_exp.relative_charge_state, 
																   test_data_exp.cycle_count), "Could not encode BatteryGeneralInfo message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message.");
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle BatteryGeneralInfo message.");
	// No encoded response
	zassert_equal(message.msg_type, INCOMING);
}


/// RequestBatteryNotifications ///

/// @brief Test that an enable==true request adds a subscriber and encodes a response.
ZTEST_F(battery_message_handler_suite, test_handle_true_gen_subcribe_request)
{
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, true), "Could not encode ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, msg_type_t::INCOMING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle ReqBatteryNotifications message.");
	zassert_equal(fixture->bat_mngr->GetSubscriberCount(GENERAL), 1);	// This fails now??
	zassert_true(fixture->bat_mngr->CpuIsSubscribed());

	// Check encoded Response message is as expected
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	RespBatteryNotifications resp_bn = RespBatteryNotifications_init_zero;
	zassert_true(fixture->msg_proto.DecodeInnerMessage(RespBatteryNotifications_fields, &resp_bn), "Could not decode RespBatteryNotifications message");
}

/// @brief Test that an enable==false does not add a subscriber (and currently no response).
ZTEST_F(battery_message_handler_suite, test_handle_false_gen_subcribe_request)
{
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, false), "Could not encode ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, msg_type_t::INCOMING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle ReqBatteryNotifications message.");

	// Check that no acknowledgement leads to message buffer being unmodified.
	zassert_equal(message.msg_type, msg_type_t::INCOMING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	ReqBatteryNotifications req_bn = ReqBatteryNotifications_init_zero;
	zassert_true(fixture->msg_proto.DecodeInnerMessage(ReqBatteryNotifications_fields, &req_bn), "Expected ReqBatteryNotifications message, but couldn't decode");
}

/// @brief Test that multiple subscribe requests from CPU does not lead to multiple subscribe callbacks being registered.
ZTEST_F(battery_message_handler_suite, test_handle_multiple_gen_subscribe_request)
{	
	for(int i = 0; i < 3; i++)
	{
		MessageBuffer message;
		zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, true), "Could not encode ReqBatteryNotifications message.");
		zassert_equal(message.msg_type, msg_type_t::INCOMING);
		zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
		zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle ReqBatteryNotifications message.");
		
		// Check encoded Response message is as expected
		zassert_equal(message.msg_type, msg_type_t::OUTGOING);
		zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
		RespBatteryNotifications resp_bn = RespBatteryNotifications_init_zero;
		zassert_true(fixture->msg_proto.DecodeInnerMessage(RespBatteryNotifications_fields, &resp_bn), "Could not decode RespBatteryNotifications message");
	}
	zassert_equal(fixture->bat_mngr->GetSubscriberCount(GENERAL), 1);
	zassert_true(fixture->bat_mngr->CpuIsSubscribed());
}


ZTEST_F(battery_message_handler_suite, test_handle_gen_resubscribe_request)
{	
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, true), "Could not encode ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, msg_type_t::INCOMING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle ReqBatteryNotifications message.");
	zassert_equal(fixture->bat_mngr->GetSubscriberCount(GENERAL), 1);
	zassert_true(fixture->bat_mngr->CpuIsSubscribed());
	// Check encoded Response message is as expected
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	RespBatteryNotifications resp_bn = RespBatteryNotifications_init_zero;
	zassert_true(fixture->msg_proto.DecodeInnerMessage(RespBatteryNotifications_fields, &resp_bn), "Could not decode RespBatteryNotifications message");

	fixture->bat_mngr->ClearSubscribers(GENERAL);
	zassert_equal(fixture->bat_mngr->GetSubscriberCount(GENERAL), 0);
	zassert_false(fixture->bat_mngr->CpuIsSubscribed());
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, true), "Could not encode ReqBatteryNotifications message.");
	message.msg_type = INCOMING;
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle ReqBatteryNotifications message.");
	zassert_equal(fixture->bat_mngr->GetSubscriberCount(GENERAL), 1);
	zassert_true(fixture->bat_mngr->CpuIsSubscribed());
	// Check encoded Response message is as expected
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	resp_bn = RespBatteryNotifications_init_zero;
	zassert_true(fixture->msg_proto.DecodeInnerMessage(RespBatteryNotifications_fields, &resp_bn), "Could not decode RespBatteryNotifications message");
}

/// ResponseBatteryNotifications ///

ZTEST_F(battery_message_handler_suite, test_handler_gen_subcribe_response)
{
	// Encode a response BatteryNotifications message
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeRespBatteryNotifications(message), "Could not encode RespBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(fixture->msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_true(fixture->bat_msg_handler->Handle(fixture->msg_proto, message), "Could not handle RespBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
}