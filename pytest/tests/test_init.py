import pytest
from libraries.general import General

all_markers = General.compose_decos(pytest.mark.init, pytest.mark.smoke, pytest.mark.nightly)

@all_markers
def test_init(test_init):
    pass