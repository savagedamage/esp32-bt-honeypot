#pragma once

// BLE sacrificial host: two jobs in one firmware.
//   1. Central probe — scan, connect to the suspect device, enumerate every
//      GATT service/characteristic, read values, subscribe to notifications,
//      and log the lot.
//   2. Honeypot peripheral — advertise a fake companion service so the suspect
//      device connects to *us* and we log everything it reads/writes.
void bleSetup();
void bleLoop();
