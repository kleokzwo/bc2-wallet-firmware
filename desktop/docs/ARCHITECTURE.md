# Architektur v0.30.0

KISS: nur `ui`, `services` und `device`. Der Desktop übernimmt Darstellung und öffentliche Kommunikation. Seed, Private Keys, 4-stellige PIN, Freigaben und spätere Signierung bleiben auf der ESP32-S3-Hardware.

USB v1 nutzt in diesem Sprint ausschließlich `PING`, `GET_INFO`, `GET_STATE` und `GET_CAPABILITIES`.
