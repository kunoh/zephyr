import pytest
from libraries.general import General

pytestmark = pytest.mark.order(1)

all_test_markers = General.get_all_markers(pytest.mark.init, pytest.mark.smoke, pytest.mark.nightly)

@all_test_markers
def test_init(test_init):
    pass