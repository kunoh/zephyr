import pytest
import json
from libraries.general import General
from libraries.switchbox_if import SwitchboxIF

pytest.g_firware_update_status = "good"

def pytest_addoption(parser):
    parser.addoption("--envfile", default="./my-env-setup.json", help="Path to environment JSON file (generated with jq -n --arg)")
    parser.addoption("--template", default="template option", help="Add more cmdline options above)")

@pytest.fixture(scope="session")
def variables(request):
    with open(request.config.getoption("--envfile")) as env:
        return json.load(env)

@pytest.fixture(scope="session")
def test_init(variables):
    ##########################################
    # Test setup
    ##########################################
    ## Assert we are in App Mode
    switch = SwitchboxIF(variables['device']['raspi_serial_pico'])
    if switch.set_mcu_boot_mode("app") is False:
         pytest.exit(f"Failed to enter MCU app mode", 3)

    ## Update FW under test
    if General.firmware_upgrade(env=variables) is False:
        print(f"Failed to upgrade with  {variables['binaries']['normal']['version']}")
        print(f"Check and do recovery")
        if General.recover_mcu(variables) is False:
            pytest.exit(f"MCU recovry fails", 5)
        pytest.exit(f"Firware update fails", 3)

    yield

    ##########################################
    # Test Teardown
    ##########################################
    if General.recover_mcu(variables) is False:
        pytest.exit(f"MCU recovry fails", 5)