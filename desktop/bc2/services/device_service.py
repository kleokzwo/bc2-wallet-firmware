from PySide6.QtCore import QObject,QThread,Signal,Slot
from bc2.device.discovery import DeviceDiscovery
class _Worker(QObject):
    finished=Signal(object)
    @Slot()
    def run(self): self.finished.emit(DeviceDiscovery().find_first())
class DeviceService(QObject):
    scan_started=Signal(); scan_finished=Signal(object)
    def __init__(self,parent=None): super().__init__(parent); self._thread=None; self._worker=None
    @Slot()
    def scan(self):
        if self._thread is not None: return
        self.scan_started.emit(); t=QThread(self); w=_Worker(); w.moveToThread(t); t.started.connect(w.run); w.finished.connect(self._done); w.finished.connect(t.quit); t.finished.connect(w.deleteLater); t.finished.connect(t.deleteLater); self._thread=t; self._worker=w; t.start()
    @Slot(object)
    def _done(self,result): self._thread=None; self._worker=None; self.scan_finished.emit(result)
