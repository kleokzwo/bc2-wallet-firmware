# v0.42.1

Receive reliability:
- Temporary `no BC2 response received` after hardware confirmation is now retried instead of aborting the Receive flow.
- This accounts for the slow E-Paper refresh temporarily blocking the firmware main loop.
- USB command timeout increased from 2 seconds to 5 seconds.
- The Desktop keeps polling until the hardware returns the already-approved receive address or the overall 120-second operation timeout expires.
