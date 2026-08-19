#pragma once
#ifdef CONFIG_BT_ENABLED
#error "BC2 SECURITY POLICY: Bluetooth/BLE must remain disabled"
#endif
void bc2_radio_policy_assert_disabled(void);
