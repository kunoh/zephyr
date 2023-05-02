
import pytest
from libraries.serial_if import MessageIF

pytestmark =  pytest.mark.nightly

@pytest.fixture(scope="module")
def msg_cli(variables):
    msg_cli = MessageIF(variables['device']['mcu_interface'], 
                  variables['device']['mcu_communication_protocol'],
                  variables['paths']['root_path']
                  )
    return msg_cli

@pytest.fixture(scope="module")
def io_msg_list(msg_cli):
    msg_list = msg_cli.get_messages()
    # Diplay massges list
    for k, v in msg_list.items():
        print(k, v)
    return msg_list

@pytest.mark.smoke
def test_io_msg_system_version(msg_cli, io_msg_list):
    print(f"Test message 'RequestVersion'")
    req_rep = msg_cli.send_recv_message("RequestVersion")
    assert "version: 2" == str(req_rep).strip().replace("\n", "")
