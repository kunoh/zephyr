import pytest
from libraries.general import General

pytestmark = pytest.mark.order(3)

@pytest.mark.smoke
def test_firmware_downgrade(variables):
    print("Test firmware downgrade")
    r =  General.firmware_upgrade(variables, variables['binaries']['recovery']['file']) 
    if r is False:
        print(f"Failed to upgrade with  {variables['binaries']['recovery']['file']}")
        print(f"Check and do recovery")
        if General.recover_mcu(variables) is False:
          pytest.exit(f"MCU recovry fails", 5)
        pytest.exit(f"Firmware update fails", 3)
    assert r

@pytest.mark.smoke
def test_firmware_upgrade(variables):
    print("Test firmware upgrade")
    r = General.firmware_upgrade(variables)
    if r is False:
      print(f"Failed to upgrade with  {variables['binaries']['normal']['version']}")
      print(f"Check and do recovery")
      if General.recover_mcu(variables) is False:
        pytest.exit(f"MCU recovry fails", 5)
      pytest.exit(f"Firmware update fails", 3)
    assert r
