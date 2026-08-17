import os
import unittest

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

try:
    from PySide6.QtWidgets import QApplication
    from bc2.ui.main_window import MainWindow
except Exception:
    QApplication = None
    MainWindow = None


@unittest.skipIf(QApplication is None, "PySide6 not available")
class UISmokeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = QApplication.instance() or QApplication([])

    def test_all_navigation_pages_exist_and_are_switchable(self):
        window = MainWindow()
        expected = {
            "setup",
            "dashboard",
            "receive",
            "send",
            "transactions",
            "device",
            "settings",
            "about",
        }
        self.assertEqual(set(window._pages), expected)

        for key in expected:
            window._navigate(key)
            self.assertIs(window._stack.currentWidget(), window._pages[key])

        window.close()


if __name__ == "__main__":
    unittest.main()
