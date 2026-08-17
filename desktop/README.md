# BC2 Cold Wallet Desktop v0.35.0

## Echter Hardware-Receive-Flow

Der Desktop erzeugt keine Empfangsadresse und kennt keinen Seed/Private Key.

Beim Klick auf **Empfangsadresse anfordern**:

1. Python sendet nur eine Adressanforderung an die Hardware.
2. Die Hardware leitet die Adresse aus ihrem Seed ab.
3. Die Hardware verlangt die 4-stellige PIN.
4. Die Adresse muss vollständig auf dem E-Paper bestätigt werden.
5. Erst nach Bestätigung wird sie in Python angezeigt.
6. Die bestätigte Adresse kann kopiert werden.

Der USB-Ablauf läuft in einem Hintergrund-Thread; die Oberfläche bleibt
während PIN und Hardware-Bestätigung bedienbar.

## Start

```bash
./run.sh
```

Für diesen Sprint ist Firmware v0.35.0 erforderlich.
