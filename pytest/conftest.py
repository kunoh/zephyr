import pytest
import json
from libraries.general import General
from libraries.switchbox_if import SwitchboxIF
from libraries.serial_if import MessageIF

pytest.g_firware_update_status = "good"


##########################################
# Command Line Arguments
##########################################
def pytest_addoption(parser):
    parser.addoption("--envfile", default="./my-env-setup.json", help="Path to environment JSON file (generated with jq -n --arg)")
    parser.addoption("--template", default="template option", help="Add more cmdline options above)")

@pytest.fixture(scope="session")
def variables(request):
    with open(request.config.getoption("--envfile")) as env:
        return json.load(env)

##########################################
# Test Setup and Teardown
##########################################
@pytest.fixture(scope="session")
def test_init(variables):
    ############## Setup ##############
    ## Assert we start in App Mode
    switch = SwitchboxIF(variables['device']['raspi_serial_pico'])
    if switch.check_usb_app_zephyr() is False:
        pytest.exit(f"Failed to validate USB ZEPHYR: Die", 3)

    ## Update FW under test
    if General.firmware_upgrade(env=variables) is False:
        print(f"Failed to upgrade with  {variables['binaries']['normal']['version']}")
        print(f"Check and do recovery")
        if General.recover_mcu(variables) is False:
            pytest.exit(f"MCU recovry fails", 5)
        pytest.exit(f"Firware update fails", 3)

    yield

    ############## Teardown ##############
    ## Assert we end in App Mode
    if switch.check_usb_app_zephyr() is False:
        pytest.exit(f"Failed to validate USB ZEPHYR: Die", 3)


##########################################
# Other
##########################################
@pytest.fixture(scope="session")
def msg_cli(variables):
    msg_cli = MessageIF(variables['device']['mcu_interface'], 
                  variables['device']['mcu_communication_protocol'],
                  variables['paths']['root_path']
                  )
    return msg_cli