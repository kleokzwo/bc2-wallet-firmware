import unittest

from bc2.device.model import DeviceInfo


class DeviceModelTests(unittest.TestCase):
    def test_info_fields_are_parsed_without_extra_complexity(self):
        device = DeviceInfo(
            port="/dev/ttyACM0",
            info=(
                "BC2 Cold Wallet 0.30.0\n"
                "Waveshare ESP32-S3-ePaper-1.54\n"
                "200x200\n"
                "revision=2"
            ),
            state=2,
            capability_flags=0x1F,
            board_revision=2,
        )

        self.assertEqual(device.device_name, "BC2 Cold Wallet 0.30.0")
        self.assertEqual(device.hardware_name, "Waveshare ESP32-S3-ePaper-1.54")
        self.assertEqual(device.display_name, "200x200")
        self.assertEqual(device.firmware_revision, "2")
        self.assertEqual(device.capabilities_text, "USB, NVS, RNG, Display, Tasten")


if __name__ == "__main__":
    unittest.main()
