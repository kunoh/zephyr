import pytest
import json
from libraries.general import General

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
    if General.firmware_upgrade(env=variables) is False:
        pytest.exit(f"Failed to upgrade with  {variables['binary']['version']}")

    yield

    ##########################################
    # Test Teardown
    ##########################################
