# BC2 Firmware v0.32.2 Recovery

- Basis: unveränderter Hardware-/USB-/Display-/Button-Unterbau aus v0.29.7
- PIN-Länge: exakt 4 numerische Stellen
- legacy v0.29.x 6-stelliger PIN-Datensatz wird einmalig entfernt
- ohne gültige 4-stellige PIN startet das Gerät direkt in `PIN ANLEGEN`
- neue 4-stellige PIN wird als Record-Version 2 gespeichert
- Factory Reset aus v0.32.0 bewusst zurückgenommen, bis Stabilität bestätigt ist
- Host-Coretests: 21/21 PASS
