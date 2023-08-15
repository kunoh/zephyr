
# import pytest
# import re, time

# pytestmark = pytest.mark.order(3)

# RE_VALUE = "[-+]?[0-9]+.*\n"

# @pytest.mark.custom
# def test_io_msg_battery_general_info(msg_cli):
#     print(f"Test message 'ReqBatteryGeneralInfo'")
#     overall_pass = True

#     req_rep = msg_cli.send_recv_message("ReqBatteryGeneralInfo")
#     print(f"Received message: {req_rep}")

#     temp = re.search(rf"temperature: {RE_VALUE}", str(req_rep))
#     temp = temp.group(0).replace('\n', '').split(':')[1].strip()
#     if not (15.0 <= float(temp) <= 35.0):
#         print(f"Temperature fails: {temp} not in range")
#         overall_pass = False

#     volt = re.search(rf"voltage: {RE_VALUE}", str(req_rep))
#     volt = volt.group(0).replace('\n', '').split(':')[1].strip()
#     if not (12.0 <= float(volt) <= 17.2):
#         print(f"Voltage fails: {volt} not in range")
#         overall_pass = False

#     curr = re.search(rf"current: {RE_VALUE}", str(req_rep))
#     curr = curr.group(0).replace('\n', '').split(':')[1].strip()
#     if not (1.0 <= float(curr) <= 8.0):
#         print(f"Current fails: {curr} not in range")
#         overall_pass = False

#     rem_cap = re.search(rf"rem_capacity: {RE_VALUE}", str(req_rep))
#     rem_cap = rem_cap.group(0).replace('\n', '').split(':')[1].strip()
#     if not (0 <= int(rem_cap) <= 65535):
#         print(f"Remaining capacity fails: {rem_cap} not in range")
#         overall_pass = False

#     cl_cnt = re.search(rf"cycle_count: {RE_VALUE}", str(req_rep))
#     cl_cnt = cl_cnt.group(0).replace('\n', '').split(':')[1].strip()
#     if not (0 <= int(cl_cnt) <= 65534):
#         print(f"Cycle count fails: {cl_cnt} not in range")
#         overall_pass = False

#     assert overall_pass

# @pytest.mark.custom
# def test_io_msg_battery_charging_info(msg_cli):
#     print(f"Test message 'ReqBatteryChargingInfo'")
#     overall_pass = True

#     time.sleep(0.1)
#     req_rep = msg_cli.send_recv_message("ReqBatteryChargingInfo")
#     print(f"Received message: {req_rep}")

#     stat = re.search(rf"status: {RE_VALUE}", str(req_rep))
#     stat = stat.group(0).replace('\n', '').split(':')[1].strip()
#     if not (int(stat) == 192):
#         print(f"Status fails: {stat} not in range")
#         overall_pass = False

#     des_chrg_curr = re.search(rf"desired_charging_current: {RE_VALUE}", str(req_rep))
#     des_chrg_curr = des_chrg_curr.group(0).replace('\n', '').split(':')[1].strip()
#     if not (1000 <= int(des_chrg_curr) <= 4000):
#         print(f"Desired charging current fails: {des_chrg_curr} not in range")
#         overall_pass = False

#     des_chrg_volt = re.search(rf"desired_charging_voltage: {RE_VALUE}", str(req_rep))
#     des_chrg_volt = des_chrg_volt.group(0).replace('\n', '').split(':')[1].strip()
#     if not (14000 <= int(des_chrg_volt) <= 16800):
#         print(f"Desired charging voltage fails: {des_chrg_volt} not in range")
#         overall_pass = False

#     rel_state_chrg = re.search(rf"relative_state_of_charge: {RE_VALUE}", str(req_rep))
#     rel_state_chrg = rel_state_chrg.group(0).replace('\n', '').split(':')[1].strip()
#     if not (20 <= int(rel_state_chrg) <= 100):
#         print(f"Relative state of charge fails: {rel_state_chrg} not in range")
#         overall_pass = False

#     chrg = re.search(rf"charging: .*", str(req_rep))
#     chrg = chrg.group(0).replace('\n', '').split(':')[1].strip()
#     if not (bool(chrg) == True):
#         print(f"Charging fails: {chrg}")
#         overall_pass = False

#     assert overall_pass


