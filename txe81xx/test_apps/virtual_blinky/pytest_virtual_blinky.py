import pytest


@pytest.mark.esp32
@pytest.mark.esp32s3
def test_virtual_blinky(dut):
    #dut.expect(r"TXE81XX virtual mode enabled")
    dut.expect(r"TXE81XX device id: 0x[0-9A-F]{2}")
    dut.expect(r"PORT0 output=0x01")
    dut.expect(r"PORT0 output=0x00")
