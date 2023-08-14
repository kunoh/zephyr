import pytest
import re
import time

pytestmark = pytest.mark.order(4)

RE_VALUE = "[-+]?[0-9]+.*\n"

@pytest.mark.smoke
def test_io_msg_get_angle(msg_cli):
    print("Test inclinometer message 'GetAngle'")
    overall_pass = True

    msg_resp = msg_cli.send_recv_message("GetAngle")
    
    # Test x angle is within +-30deg of 0. Inclinometer is horizontal.
    x_angle = re.search(rf"x_angle: {RE_VALUE}", str(msg_resp))
    x_angle = x_angle.group(0).replace('\n', '').split(':')[1].strip()
    print(f"x_angle: \n {x_angle}")
    if not (-30.0 <= float(x_angle) <= 30.0):
        print(f"Inclinometer angle fails: X-angle '{x_angle}' not in range")
        overall_pass = False

    # Test y angle is within +-30deg of 0. Inclinometer is horizontal.
    y_angle = re.search(rf"y_angle: {RE_VALUE}", str(msg_resp))
    y_angle = y_angle.group(0).replace('\n', '').split(':')[1].strip()
    print(f"y_angle: \n {y_angle}")
    if not (-30.0 <= float(y_angle) <= 30.0):
        print(f"Inclinometer angle fails: Y-angle '{y_angle}' not in range")
        overall_pass = False

    # Test z angle is within +-30deg of -90deg. Inclinometer is horizontal, but upside down. (each axis measures from 90 to -90).
    z_angle = re.search(rf"z_angle: {RE_VALUE}", str(msg_resp))
    z_angle = z_angle.group(0).replace('\n', '').split(':')[1].strip()
    print(f"z_angle: \n {z_angle}")
    if not (-90.0 <= float(z_angle) <= -60.0):
        print(f"Inclinometer angle fails: Z-angle '{z_angle}' not in range")
        overall_pass = False

    # Test that a new measurement is different from the previous one.
    time.sleep(2.5)
    msg_resp_new = msg_cli.send_recv_message("GetAngle")

    print(f"msg_resp_new: \n{msg_resp_new}")
 
    x_angle_new = re.search(rf"x_angle: {RE_VALUE}", str(msg_resp_new))
    x_angle_new = x_angle_new.group(0).replace('\n', '').split(':')[1].strip()
    print(f"x_angle_new: \n {x_angle_new}")
    y_angle_new = re.search(rf"y_angle: {RE_VALUE}", str(msg_resp_new))
    y_angle_new = y_angle_new.group(0).replace('\n', '').split(':')[1].strip()
    print(f"y_angle_new: \n {y_angle_new}")
    z_angle_new = re.search(rf"z_angle: {RE_VALUE}", str(msg_resp_new))
    z_angle_new = z_angle_new.group(0).replace('\n', '').split(':')[1].strip()
    print(f"z_angle_new: \n {z_angle_new}")

    if (float(x_angle) == float(x_angle_new))  and  (float(y_angle) == float(y_angle_new))  and  (float(z_angle) == float(z_angle_new)):
        print(f"Inclinometer angle fails: X-angle did not change on new read: {x_angle}=={x_angle_new}")
        print(f"Inclinometer angle fails: Y-angle did not change on new read: {y_angle}=={y_angle_new}")
        print(f"Inclinometer angle fails: Z-angle did not change on new read: {z_angle}=={z_angle_new}")
        overall_pass = False    
        # To make sure we wern't just unlucky and two measurements of one angle were identical, all angles have to be the same in order to fail.

    assert overall_pass


@pytest.mark.smoke
def test_io_msg_set_sample(msg_cli):
    print("Test inclinometer message 'SetSamplePeriod'")
    overall_pass = True

    msg_resp = msg_cli.send_recv_message("GetSamplePeriod")

    sample_period_first = re.search(rf"period: {RE_VALUE}", str(msg_resp))
    sample_period_first = sample_period_first.group(0).replace('\n', '').split(':')[1].strip()
    print(f"sample_period_first: \n {sample_period_first}")
    
    msg_resp = msg_cli.send_recv_message("SetSamplePeriod", int(sample_period_first)+1)

    msg_resp = msg_cli.send_recv_message("GetSamplePeriod")

    sample_period_second = re.search(rf"period: {RE_VALUE}", str(msg_resp))
    sample_period_second = sample_period_second.group(0).replace('\n', '').split(':')[1].strip()
    print(f"sample_period_second: \n {sample_period_second}")

    if int(sample_period_first) == int(sample_period_second):
        print(f"Inclinometer get/set sample fails. sample period did not change.")
        overall_pass = False

    # Reset sample period to default.
    msg_resp = msg_cli.send_recv_message("SetSamplePeriod", int(sample_period_first))

    assert overall_pass



    


