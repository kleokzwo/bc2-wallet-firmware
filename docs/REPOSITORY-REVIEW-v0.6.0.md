# Repository-Prüfung vor v0.7.0

Der Stand v0.6.0 enthielt einen begrenzten PSBT-Hüllenprüfer. Er erkannte Magic, Größenlimits und das Vorhandensein einer globalen unsigned transaction, interpretierte aber weder die Transaktion noch Inputs, Outputs, Beträge, Gebühren oder Wechselgeld. Der Qt-Dialog zeigte daher nur Strukturinformationen.

Für v0.7.0 wurde die bestehende Architektur beibehalten. Der Parser bleibt im plattformunabhängigen C17-Core. Qt liefert ausschließlich bekannte öffentliche Watch-only-Scripts zur Eigentums- und Wechselgeldprüfung und stellt das Core-Ergebnis dar.
