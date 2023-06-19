/**
 * @file test_handlers.cpp
 *
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

/// @brief Test HandleBatteryGeneralInfo 
ZTEST_F(battery_message_handler_suite, test_handle_battery_gen_info)
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
	zassert_true(BatteryMessageEncoder::EncodeBatteryGeneralInfo(message, test_data_exp.temp, test_data_exp.volt, test_data_exp.current,
																   test_data_exp.remaining_capacity,
																   test_data_exp.cycle_count), "Could not encode BatteryGeneralInfo message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle BatteryGeneralInfo message.");
	// No encoded response
	zassert_equal(message.msg_type, INCOMING);
}

/// @brief Test HandleBatteryChargingInfo 
ZTEST_F(battery_message_handler_suite, test_handle_battery_chg_info)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	// Encode a BatteryGeneralInfo message
	MessageBuffer message;
	BatteryChargingData test_data_exp;
	zassert_true(BatteryMessageEncoder::EncodeBatteryChargingInfo(message, test_data_exp.status, test_data_exp.des_chg_current, test_data_exp.des_chg_volt, test_data_exp.relative_charge_state, test_data_exp.charging), "Could not encode BatteryChargingInfo message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryChargingInfo Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle BatteryChargingInfo message.");

	// No encoded response
	zassert_equal(message.msg_type, INCOMING);
}

/// @brief Test message handling of ReqBatteryGeneralInfo.
ZTEST_F(battery_message_handler_suite, test_handle_request_battery_gen_info)
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

	bat_mngr.StartSampling(GENERAL, 20, 60000);
	k_msleep(30);

	// Encode a RequestBatteryGeneralInfo message.
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryGeneralInfo(message), "Could not encode ReqBatteryGeneralInfo message.");
	zassert_equal(message.msg_type, INCOMING);

	// 1. Assert decodable
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryGeneralInfo message.");

	// 2. & 3. Check encoded Response message is as expected.
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryGeneralInfo Outer message");

	BatteryGeneralInfo bgi = BatteryGeneralInfo_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(BatteryGeneralInfo_fields, &bgi), "Could not decode BatteryGeneralInfo message");

	zassert_equal(bgi.temperature, test_data_exp.temp);
	zassert_equal(bgi.voltage, test_data_exp.volt);
	zassert_equal(bgi.current, test_data_exp.current);
	zassert_equal(bgi.rem_capacity, test_data_exp.remaining_capacity);
	zassert_equal(bgi.cycle_count, test_data_exp.cycle_count);

	// If the sampling timer is stopped we expect request-responses with invalid data.
	bat_mngr.StopSampling(GENERAL);
}

/// @brief Test message handling of ReqBatteryChargingInfo.
ZTEST_F(battery_message_handler_suite, test_handle_request_battery_chg_info)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);

	// Setup some dummy variables and assign them to the mocked battery
    BatteryChargingData test_data_exp;
	test_data_exp.des_chg_current = 5500;
	test_data_exp.des_chg_volt = 16500;
	test_data_exp.status = 0x80000000;
	test_data_exp.relative_charge_state = 80;

    BatteryMock test_battery_mock;
	test_battery_mock.SetTestChargingCurrent(test_data_exp.des_chg_current);
	test_battery_mock.SetTestChargingVoltage(test_data_exp.des_chg_volt);
	test_battery_mock.SetTestStatus(test_data_exp.status);
	test_battery_mock.SetTestRelChargeState(test_data_exp.relative_charge_state);

	BatteryChargerMock test_charger_mock;
    BatteryManager bat_mngr = BatteryManager(test_battery_mock, test_charger_mock);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	bat_mngr.StartSampling(CHARGING, 20, CHARGING_PERIOD_MSEC);
	k_msleep(30);

	// Encode a RequestBatteryChargingInfo message.
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryChargingInfo(message), "Could not encode ReqBatteryChargingInfo message.");
	zassert_equal(message.msg_type, INCOMING);

	// 1. Assert decodable
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryChargingInfo Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryChargingInfo message.");

	// 2. & 3. Check encoded Response message is as expected.
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryChargingInfo Outer message");
	BatteryChargingInfo bci = BatteryChargingInfo_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(BatteryChargingInfo_fields, &bci), "Could not decode BatteryChargingInfo message");

	zassert_equal(bci.desired_charging_current, test_data_exp.des_chg_current);
	zassert_equal(bci.desired_charging_voltage, test_data_exp.des_chg_volt);
	zassert_equal(bci.status, test_data_exp.status);
	zassert_equal(bci.relative_state_of_charge, test_data_exp.relative_charge_state);
	zassert_equal(bci.charging, false);

	// If the sampling timer is stopped we expect request-responses with invalid data.
	bat_mngr.StopSampling(CHARGING);
}

/// @brief Test successful handling of ReqBatteryNotifications
ZTEST_F(battery_message_handler_suite, test_handle_req_battery_notifications)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	bat_mngr.Init();
	bat_mngr.StopSampling(GENERAL);
	bat_mngr.StopSampling(CHARGING);

	// Encode a SetInstallationMode message
	MessageBuffer message;
	char sub[4] = "CPU";

	// General battery data
	char type[8] = "GENERAL";
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, sub, type), "Could not encode ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");

	zassert_equal(bat_mngr.GetSubscriberCbCount(GENERAL), 0);
	zassert_false(bat_mngr.IsSubscribed(sub, GENERAL));
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryNotifications message.");
	zassert_true(bat_mngr.IsSubscribed(sub, GENERAL));
	zassert_equal(bat_mngr.GetSubscriberCbCount(GENERAL), 1);
	
	// Check that handling encoded a response message
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	RespBatteryNotifications bn = RespBatteryNotifications_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(RespBatteryNotifications_fields, &bn), "Could not decode RespBatteryNotifications message");

	// Battery charging data
	char type_2[9] = "CHARGING";
	message.msg_type = INCOMING;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, sub, type_2), "Could not encode ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");

	zassert_equal(bat_mngr.GetSubscriberCbCount(CHARGING), 0);
	zassert_false(bat_mngr.IsSubscribed(sub, CHARGING));
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryNotifications message.");
	zassert_true(bat_mngr.IsSubscribed(sub, CHARGING));
	zassert_equal(bat_mngr.GetSubscriberCbCount(CHARGING), 1);
	
	// Check that handling encoded a response message
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	bn = RespBatteryNotifications_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(RespBatteryNotifications_fields, &bn), "Could not decode RespBatteryNotifications message");
}

/// @brief Test handling of ReqBatteryNotifications when subscriber is already subscribed
ZTEST_F(battery_message_handler_suite, test_handle_req_battery_notifications_already_subscribed)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	bat_mngr.Init();
	bat_mngr.StopSampling(GENERAL);
	bat_mngr.StopSampling(CHARGING);

	// Encode a SetInstallationMode message
	MessageBuffer message;
	char sub[4] = "CPU";

	// General battery data
	char type[8] = "GENERAL";
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, sub, type), "Could not encode ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_equal(bat_mngr.GetSubscriberCbCount(GENERAL), 0);
	zassert_false(bat_mngr.IsSubscribed(sub, GENERAL));
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryNotifications message.");

	// Encode again
	message.msg_type = INCOMING;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, sub, type), "Could not encode ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryNotifications message.");	// Subscribe again
	zassert_true(bat_mngr.IsSubscribed(sub, GENERAL));
	zassert_equal(bat_mngr.GetSubscriberCbCount(GENERAL), 1);
	// Check that handling a repeated request still returns an ACK
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	RespBatteryNotifications bn = RespBatteryNotifications_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(RespBatteryNotifications_fields, &bn), "Could not decode RespBatteryNotifications message");
	
	// Battery charging data
	char type_2[9] = "CHARGING";
	message.msg_type = INCOMING;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, sub, type_2), "Could not encode ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_equal(bat_mngr.GetSubscriberCbCount(CHARGING), 0);
	zassert_false(bat_mngr.IsSubscribed(sub, CHARGING));
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryNotifications message.");

	// Encode again
	message.msg_type = INCOMING;
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, sub, type_2), "Could not encode ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryNotifications message.");	// Subscribe again
	zassert_true(bat_mngr.IsSubscribed(sub, CHARGING));
	zassert_equal(bat_mngr.GetSubscriberCbCount(CHARGING), 1);
	// Check that handling a repeated request still returns an ACK
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message");
	bn = RespBatteryNotifications_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(RespBatteryNotifications_fields, &bn), "Could not decode RespBatteryNotifications message");
}

/// @brief Test handling of ReqBatteryNotifications when subscriber or data type is unknown does not produce an ACK
ZTEST_F(battery_message_handler_suite, test_handle_req_battery_notifications_no_ack)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	bat_mngr.Init();
	bat_mngr.StopSampling(GENERAL);
	bat_mngr.StopSampling(CHARGING);

	// Encode a SetInstallationMode message
	MessageBuffer message;
	char sub[4] = "UPC";
	char type[8] = "GENERAL";

	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, sub, type), "Could not encode ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_false(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);

	char sub_2[4] = "CPU";
	char type_2[7] = "FOOBAR";
	zassert_true(BatteryMessageEncoder::EncodeReqBatteryNotifications(message, sub_2, type_2), "Could not encode ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode BatteryNotifications Outer message.");
	zassert_false(bat_msg_handler.Handle(msg_proto, message), "Could not handle ReqBatteryNotifications message.");
	zassert_equal(message.msg_type, INCOMING);
}

/// @brief Test RespBatteryNotifications 
ZTEST_F(battery_message_handler_suite, test_handle_subcribe_response)
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

/// @brief Test SetInstallationMode 
ZTEST_F(battery_message_handler_suite, test_handle_set_installation_mode)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	bat_mngr.Init();
	bat_mngr.StopSampling(GENERAL);
	bat_mngr.StopSampling(CHARGING);
	zassert_equal(bat_mngr.GetInstallationMode(), INSTALLATION_MODE_DEFAULT);

	// Encode a SetInstallationMode message
	MessageBuffer message;
	char mode_1[6] = "FIXED";
	zassert_true(BatteryMessageEncoder::EncodeSetInstallationMode(message, mode_1), "Could not encode SetInstallationMode message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode InstallationMode Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle SetInstallationMode message.");
	// Check encoded Response message is as expected.
	zassert_equal(bat_mngr.GetInstallationMode(), mode_1);
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode InstallationMode Outer message");
	RespInstallationMode im = RespInstallationMode_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(RespInstallationMode_fields, &im), "Could not decode RespInstallationMode message");

	char mode_2[7] = "MOBILE";
	message.msg_type = INCOMING;
	zassert_true(BatteryMessageEncoder::EncodeSetInstallationMode(message, mode_2), "Could not encode SetInstallationMode message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode InstallationMode Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle SetInstallationMode message.");
	// Check encoded Response message is as expected.
	zassert_equal(bat_mngr.GetInstallationMode(), mode_2);
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode InstallationMode Outer message");
	im = RespInstallationMode_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(RespInstallationMode_fields, &im), "Could not decode RespInstallationMode message");
}


/// @brief Test SetInstallationMode fails on trying to set unknown installation mode
ZTEST_F(battery_message_handler_suite, test_handle_set_installation_mode_unknown)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	bat_mngr.Init();
	bat_mngr.StopSampling(GENERAL);
	bat_mngr.StopSampling(CHARGING);
	zassert_equal(bat_mngr.GetInstallationMode(), INSTALLATION_MODE_DEFAULT);

	// Encode a SetInstallationMode message
	MessageBuffer message;
	char mode_1[11] = "BOGUS_MODE";
	zassert_true(BatteryMessageEncoder::EncodeSetInstallationMode(message, mode_1), "Could not encode SetInstallationMode message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode InstallationMode Outer message.");
	zassert_false(bat_msg_handler.Handle(msg_proto, message), "Could not handle SetInstallationMode message.");
	// Check encoded Response message is as expected.
	zassert_equal(bat_mngr.GetInstallationMode(), INSTALLATION_MODE_DEFAULT);
	zassert_equal(message.msg_type, msg_type_t::INCOMING);

}

/// @brief Test RespInstallationMode 
ZTEST_F(battery_message_handler_suite, test_handle_installation_mode_response)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	// Encode a response InstallationMode message
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeRespInstallationMode(message), "Could not encode RespInstallationMode message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode InstallationMode Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle RespInstallationMode message.");
	zassert_equal(message.msg_type, INCOMING);
}

/// @brief Test SetInstallationMode 
ZTEST_F(battery_message_handler_suite, test_handle_set_mode_charging_limit)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	bat_mngr.Init();
	bat_mngr.StopSampling(GENERAL);
	bat_mngr.StopSampling(CHARGING);

	// Encode a SetInstallationMode message
	MessageBuffer message;
	int test_chg_limit = CHARGING_REL_CHG_STATE_DEFAULT - 1;
	char mode_1[7] = "MOBILE";

	// Check that test preconditions are valid
	int32_t limit_out = 0;
	zassert_ok(bat_mngr.GetModeChargingLimit(mode_1, limit_out));
	zassert_equal(limit_out, CHARGING_REL_CHG_STATE_DEFAULT);

	zassert_true(BatteryMessageEncoder::EncodeSetModeChargingLimit(message, mode_1, test_chg_limit), "Could not encode SetModeChargingLimit message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode ModeChargingLimit Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle SetModeChargingLimit message.");

	// Check that charging limit has changed
	limit_out = 0;
	zassert_ok(bat_mngr.GetModeChargingLimit(mode_1, limit_out));
	zassert_equal(limit_out, test_chg_limit);

	// Check that handling encoded a response message
	zassert_equal(message.msg_type, msg_type_t::OUTGOING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode ModeChargingLimit Outer message");
	RespModeChargingLimit mcl = RespModeChargingLimit_init_zero;
	zassert_true(msg_proto.DecodeInnerMessage(RespModeChargingLimit_fields, &mcl), "Could not decode RespModeChargingLimit message");
}

/// @brief Test that handling SetModeChargingLimit with OOR limits produces no ACK
ZTEST_F(battery_message_handler_suite, test_handle_set_mode_charging_limit_ERANGE)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	bat_mngr.Init();
	bat_mngr.StopSampling(GENERAL);
	bat_mngr.StopSampling(CHARGING);

	// Encode a SetInstallationMode message
	MessageBuffer message;
	int test_chg_limit = CHARGING_REL_CHG_STATE_MAX + 1;
	char mode_1[7] = "MOBILE";

	// Check that test preconditions are valid
	int32_t limit_out = 0;
	zassert_ok(bat_mngr.GetModeChargingLimit(mode_1, limit_out));
	zassert_equal(limit_out, CHARGING_REL_CHG_STATE_DEFAULT);

	zassert_true(BatteryMessageEncoder::EncodeSetModeChargingLimit(message, mode_1, test_chg_limit), "Could not encode SetModeChargingLimit message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode ModeChargingLimit Outer message.");
	zassert_false(bat_msg_handler.Handle(msg_proto, message));

	// Check that charging limit has not changed
	limit_out = 0;
	zassert_ok(bat_mngr.GetModeChargingLimit(mode_1, limit_out));
	zassert_equal(limit_out, CHARGING_REL_CHG_STATE_DEFAULT);
	zassert_equal(message.msg_type, msg_type_t::INCOMING);
}

/// @brief Test RespModeChargingLimit
ZTEST_F(battery_message_handler_suite, test_handle_mode_charging_limit_response)
{
	MessageProto msg_proto;
	MessageDispatcher dispatcher;
	UsbHidMock usb_mock;
	BatteryMock test_battery_mock;
	BatteryChargerMock test_battery_charger_mock;
	BatteryManager bat_mngr(test_battery_mock, test_battery_charger_mock);
	MessageManager msg_mngr(usb_mock, msg_proto, dispatcher);
	BatteryMessageHandlerImpl bat_msg_handler(bat_mngr, msg_mngr);

	// Encode a response ModeChargingLimit message
	MessageBuffer message;
	zassert_true(BatteryMessageEncoder::EncodeRespModeChargingLimit(message), "Could not encode RespModeChargingLimit message.");
	zassert_equal(message.msg_type, INCOMING);
	zassert_true(msg_proto.DecodeOuterMessage(message), "Could not decode ModeChargingLimit Outer message.");
	zassert_true(bat_msg_handler.Handle(msg_proto, message), "Could not handle RespModeChargingLimit message.");
	zassert_equal(message.msg_type, INCOMING);
}