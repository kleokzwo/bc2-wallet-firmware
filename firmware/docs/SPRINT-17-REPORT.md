# Sprint-17-Abschlussbericht – v0.22.0

## Ursache

Das Waveshare-V2-Panel setzte BUSY korrekt auf HIGH. Die bisherige Warteschleife verwendete jedoch `pdMS_TO_TICKS(5)`. Bei einer FreeRTOS-Tickrate von 100 Hz ergibt diese Umrechnung 0 Ticks. Dadurch lief die Schleife ohne reale Wartezeit 3000-mal durch und meldete den konfigurierten 15-Sekunden-Timeout bereits nach rund 30 ms.

## Korrektur

- echte Zeitmessung mit `esp_timer_get_time()`
- mindestens ein realer FreeRTOS-Tick pro BUSY-Prüfung
- aussagekräftige Timeoutdauer im Log
- Version 0.22.0

## Erwarteter Hardwaretest

Ein Vollrefresh muss nun mehrere Sekunden auf BUSY warten und anschließend `Official Waveshare full refresh completed` melden.
