/**
 * @file test_handlers.cpp
 *
 * @author CJO
 * @brief Test suite responsible for testing the functionality of BatteryMessageHandlerImpl when processing Protobuf messages.
 */


#include <zephyr/ztest.h>

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


struct battery_message_handler_suite_fixture {
	battery_message_handler_suite_fixture()
	: msg_proto()
	, dispatcher()
	, usb_mock()
	, test_battery_mock()
	, test_battery_charger_mock()
	, bat_mngr(test_battery_mock, test_battery_charger_mock)
	, msg_mngr(usb_mock, msg_proto, dispatcher)
	, bat_msg_handler(bat_mngr, msg_mngr) {}
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr;
	MessageManager msg_mngr;
	BatteryMessageHandlerImpl bat_msg_handler;
};

ZTEST_SUITE(battery_message_handler_suite, NULL, NULL, NULL, NULL, NULL);


/// BatteryInfo ///

/// RequestBatteryInfo ///

/// @brief Test message handling of RequestBatteryInfo.
/// Test that:
/// 1. Message can be decoded.
/// 2. A response is encoded.
/// 3. Handling involves triggering and getting battery sample. Response should contain sample data.
ZTEST_F(battery_message_handler_suite, test_handler_request_battery_info)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);

	// Setup some dummy variables and assign them to the mocked battery
    BatteryGeneralData test_data_exp;
    test_data_exp.temp = 20.0;
    test_data_exp.volt = 16.0;
    test_data_exp.current = 5.5;
    test_data_exp.remaining_capacity = 5000;
    test_data_exp.cycle_count = 120;

    BatteryMock test_battery_mock;
    test_battery_mock.SetTestTemp(test_data_exp.temp);
    test_battery_mock.SetTestVolt(test_data_exp.volt);
    test_battery_mock.SetTestCurrent(test_data_exp.current);
    test_battery_mock.SetTestRemCap(test_data_exp.remaining_capacity);
    test_battery_mock.SetTestCycleCount(test_data_exp.cycle_count);

	BatteryChargerMock test_charger_mock;
    BatteryManager bat_mngr = BatteryManager(test_battery_mock, test_charger_mock);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	// Encode a RequestBatteryInfo message.
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryGeneralInfo(message), "Could not encode ReqBatteryGeneralInfo message.");
	zassert_equal(message.msg_type, INCOMING);

	// 1. Assert decodable
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryGeneralInfo message.");

	// 2. & 3. Check encoded Response message is as expected.
	// When sampling hasn't been started we expect default invalid values.
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message");
	BatteryGeneralInfo bgi = BatteryGeneralInfo_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(BatteryGeneralInfo_fields, &bgi), "Could not decode BatteryGeneralInfo message");
	zassert_equal(bgi.temperature, DEFAULT_INVALID_BAT_FLOAT);
	zassert_equal(bgi.voltage, DEFAULT_INVALID_BAT_FLOAT);
	zassert_equal(bgi.current, DEFAULT_INVALID_BAT_FLOAT);
	zassert_equal(bgi.rem_capacity, DEFAULT_INVALID_BAT_INT);
	zassert_equal(bgi.cycle_count, DEFAULT_INVALID_BAT_INT);

	bat_mngr.StartSampling(GENERAL, 20, 60000);
	k_msleep(30);

	message.msg_type = INCOMING;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryGeneralInfo(message), "Could not encode ReqBatteryGeneralInfo message.");
	zassert_equal(message.msg_type, INCOMING);

	// 1. Assert decodable
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryGeneralInfo message.");

	// 2. & 3. Check encoded Response message is as expected.
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message");
	bgi = BatteryGeneralInfo_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(BatteryGeneralInfo_fields, &bgi), "Could not decode BatteryGeneralInfo message");

	zassert_equal(bgi.temperature, test_data_exp.temp);
	zassert_equal(bgi.voltage, test_data_exp.volt);
	zassert_equal(bgi.current, test_data_exp.current);
	zassert_equal(bgi.rem_capacity, test_data_exp.remaining_capacity);
	zassert_equal(bgi.cycle_count, test_data_exp.cycle_count);

	// If the sampling timer is stopped we expect request-responses with invalid data.
	bat_mngr.StopSampling(GENERAL);
	message.msg_type = INCOMING;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryGeneralInfo(message), "Could not encode ReqBatteryGeneralInfo message.");
	zassert_equal(message.msg_type, INCOMING);

	// 1. Assert decodable
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryGeneralInfo message.");

	// 2. & 3. Check encoded Response message is as expected.
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message");
	bgi = BatteryGeneralInfo_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(BatteryGeneralInfo_fields, &bgi), "Could not decode BatteryGeneralInfo message");

	zassert_equal(bgi.temperature, DEFAULT_INVALID_BAT_FLOAT, "%f - %f", bgi.temperature, DEFAULT_INVALID_BAT_FLOAT);
	zassert_equal(bgi.voltage, DEFAULT_INVALID_BAT_FLOAT);
	zassert_equal(bgi.current, DEFAULT_INVALID_BAT_FLOAT);
	zassert_equal(bgi.rem_capacity, DEFAULT_INVALID_BAT_INT);
	zassert_equal(bgi.cycle_count, DEFAULT_INVALID_BAT_INT);
}

ZTEST_F(battery_message_handler_suite, test_handler_battery_info)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	// Encode a BatteryInfo message
	MessageBuffer message;
	BatteryGeneralData test_data_exp;
    test_data_exp.temp = 20.0;
    test_data_exp.volt = 16.0;
    test_data_exp.current = 5.5;
    test_data_exp.remaining_capacity = 5000;
    test_data_exp.cycle_count = 120;
	zassert_true(BatteryMessageEncoder::EncodeBatteryGeneralInfo(message, test_data_exp.temp, test_data_exp.volt, test_data_exp.current,
																   test_data_exp.remaining_capacity,
																   test_data_exp.cycle_count), "Could not encode BatteryGeneralInfo message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle BatteryGeneralInfo message.");
	// No encoded response
	zassert_equal(message.msg_type, INCOMING);
}

/// RequestBatteryNotifications ///

/// @brief Test that an enable==true request adds a subscriber and encodes a response.
ZTEST_F(battery_message_handler_suite, test_handle_true_gen_subcribe_request)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, true), "Could not encode ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, msg_type_t::INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");

	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryNotifications message.");
	zassert_equal(bat_mngr.GetSubscriberCount(GENERAL), 1);	// This fails now??
	zassert_true(bat_mngr.CpuIsSubscribed());

	// Check encoded Response message is as expected
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	RespBatteryNotifications resp_bn = RespBatteryNotifications_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(RespBatteryNotifications_fields, &resp_bn), "Could not decode RespBatteryNotifications message");
}

/// @brief Test that an enable==false does not add a subscriber (and currently no response).
ZTEST_F(battery_message_handler_suite, test_handle_false_gen_subcribe_request)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, false), "Could not encode ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, msg_type_t::INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryNotifications message.");

	// Check that no acknowledgement leads to message buffer being unmodified.
	zassert_equal(message.msg_type, msg_type_t::INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	ReqBatteryNotifications req_bn = ReqBatteryNotifications_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(ReqBatteryNotifications_fields, &req_bn), "Expected ReqBatteryNotifications message, but couldn't decode");
}

/// @brief Test that multiple subscribe requests from CPU does not lead to multiple subscribe callbacks being registered.
ZTEST_F(battery_message_handler_suite, test_handle_multiple_gen_subscribe_request)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	for(int i = 0; i < 3; i++)
	{
		MessageBuffer message;
		zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, true), "Could not encode ReqBatteryNotifications message.");
		zassert_equal(message.msg_type, msg_type_t::INCOMING);
		zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
		zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryNotifications message.");

		// Check encoded Response message is as expected
		zassert_equal(message.msg_type, msg_type_t::OUTGOING);
		zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
		RespBatteryNotifications resp_bn = RespBatteryNotifications_init_zero;
		zassert_true(msg_proto.DecodeInnerMessage(RespBatteryNotifications_fields, &resp_bn), "Could not decode RespBatteryNotifications message");
	}
	zassert_equal(bat_mngr.GetSubscriberCount(GENERAL), 1);
	zassert_true(bat_mngr.CpuIsSubscribed());
}


ZTEST_F(battery_message_handler_suite, test_handle_gen_resubscribe_request)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, true), "Could not encode ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, msg_type_t::INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryNotifications message.");
	zassert_equal(bat_mngr.GetSubscriberCount(GENERAL), 1);
	zassert_true(bat_mngr.CpuIsSubscribed());
	// Check encoded Response message is as expected
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	RespBatteryNotifications resp_bn = RespBatteryNotifications_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(RespBatteryNotifications_fields, &resp_bn), "Could not decode RespBatteryNotifications message");

	bat_mngr.ClearSubscribers(GENERAL);
	zassert_equal(bat_mngr.GetSubscriberCount(GENERAL), 0);
	zassert_false(bat_mngr.CpuIsSubscribed());
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, true), "Could not encode ReqBatteryNotifications message.");
	message.msg_type = INCOMING;
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryNotifications message.");
	zassert_equal(bat_mngr.GetSubscriberCount(GENERAL), 1);
	zassert_true(bat_mngr.CpuIsSubscribed());
	// Check encoded Response message is as expected
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	resp_bn = RespBatteryNotifications_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(RespBatteryNotifications_fields, &resp_bn), "Could not decode RespBatteryNotifications message");
}

/// ResponseBatteryNotifications ///

ZTEST_F(battery_message_handler_suite, test_handler_gen_subcribe_response)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	// Encode a response BatteryNotifications message
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeRespBatteryNotifications(message), "Could not encode RespBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle RespBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
}