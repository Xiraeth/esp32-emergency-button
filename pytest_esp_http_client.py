import logging
from pathlib import Path

from pytest_embedded import Dut
from pytest_embedded_idf.utils import idf_parametrize


@idf_parametrize('target', ['esp32s3'], indirect=['target'])
def test_emergency_button_boots_and_waits_for_button(dut: Dut) -> None:
    assert dut.app.binary_path is not None

    binary_file = Path(dut.app.binary_path) / 'esp_http_client.bin'
    bin_size = binary_file.stat().st_size
    logging.info(f'esp_http_client_bin_size: {bin_size // 1024}KB')

    dut.expect('Connected to network', timeout=60)
    dut.expect(r'Waiting for button press on GPIO4', timeout=10)
