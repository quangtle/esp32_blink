#ifndef BT_H
#define BT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------
 * Register map: DPORT (system control)
 * ------------------------------------------------------------------ */
#define DPORT_BASE              0x3FF48000UL
#define DPORT_PERIP_CLK_EN_REG  (*(volatile uint32_t *)(DPORT_BASE + 0x20))
#define DPORT_PERIP_RST_EN_REG  (*(volatile uint32_t *)(DPORT_BASE + 0x28))

#define DPORT_BT_CLK_EN         (1u << 3)
#define DPORT_BT_RST            (1u << 3)
#define DPORT_BLE_CLK_EN        (1u << 24)
#define DPORT_BLE_RST           (1u << 24)

/* ------------------------------------------------------------------
 * Register map: Bluetooth Controller
 * ------------------------------------------------------------------ */
#define BT_CONTROLLER_BASE      0x3FFE0000UL

/* --- BT_LC (Link Controller) --- */
#define BT_LC_BASE              0x3FFE0000UL
#define BT_LC_CTRL_REG          (*(volatile uint32_t *)(BT_LC_BASE + 0x00))
#define BT_LC_CONF_REG          (*(volatile uint32_t *)(BT_LC_BASE + 0x04))
#define BT_LC_STATE_REG         (*(volatile uint32_t *)(BT_LC_BASE + 0x08))

#define BT_LC_CTRL_RESET        (1u << 0)
#define BT_LC_CTRL_ENABLE       (1u << 1)

/* --- BT_BB (Baseband) --- */
#define BT_BB_BASE              0x3FFE0C00UL
#define BT_BB_PWR_CTRL_REG      (*(volatile uint32_t *)(BT_BB_BASE + 0x00))
#define BT_BB_CONF_REG          (*(volatile uint32_t *)(BT_BB_BASE + 0x04))
#define BT_BB_INT_STA_REG       (*(volatile uint32_t *)(BT_BB_BASE + 0x08))
#define BT_BB_INT_ENA_REG       (*(volatile uint32_t *)(BT_BB_BASE + 0x0C))
#define BT_BB_CMD_REG           (*(volatile uint32_t *)(BT_BB_BASE + 0x10))
#define BT_BB_STATUS_REG        (*(volatile uint32_t *)(BT_BB_BASE + 0x14))

#define BT_BB_PWR_UP            (1u << 0)
#define BT_BB_PWR_BT            (1u << 1)
#define BT_BB_PWR_BLE           (1u << 2)

/* --- BT_RF --- */
#define BT_RF_BASE              0x3FFE1000UL
#define BT_RF_PWR_CTRL_REG      (*(volatile uint32_t *)(BT_RF_BASE + 0x00))
#define BT_RF_CONF_REG          (*(volatile uint32_t *)(BT_RF_BASE + 0x04))

#define BT_RF_PWR_UP            (1u << 0)

/* --- HCI (Host Controller Interface) --- */
#define BT_HCI_BASE             0x3FFE2000UL
#define BT_HCI_CTRL_REG         (*(volatile uint32_t *)(BT_HCI_BASE + 0x00))
#define BT_HCI_CMD_REG          (*(volatile uint32_t *)(BT_HCI_BASE + 0x04))
#define BT_HCI_EVT_REG          (*(volatile uint32_t *)(BT_HCI_BASE + 0x08))
#define BT_HCI_DATA_OUT_REG     (*(volatile uint32_t *)(BT_HCI_BASE + 0x0C))
#define BT_HCI_DATA_IN_REG      (*(volatile uint32_t *)(BT_HCI_BASE + 0x10))
#define BT_HCI_INT_STA_REG      (*(volatile uint32_t *)(BT_HCI_BASE + 0x14))
#define BT_HCI_INT_ENA_REG      (*(volatile uint32_t *)(BT_HCI_BASE + 0x18))
#define BT_HCI_FIFO_STA_REG     (*(volatile uint32_t *)(BT_HCI_BASE + 0x1C))

#define BT_HCI_CTRL_EN          (1u << 0)
#define BT_HCI_CTRL_RESET       (1u << 1)
#define BT_HCI_INT_CMD          (1u << 0)
#define BT_HCI_INT_EVT          (1u << 1)
#define BT_HCI_INT_DATA_OUT     (1u << 2)
#define BT_HCI_INT_DATA_IN      (1u << 3)
#define BT_HCI_FIFO_CMD_RDY     (1u << 0)
#define BT_HCI_FIFO_EVT_RDY     (1u << 8)
#define BT_HCI_FIFO_DATA_IN_RDY (1u << 16)

/* --- BLE Core --- */
#define BT_BLE_BASE             0x3FFE4000UL
#define BT_BLE_CTRL_REG         (*(volatile uint32_t *)(BT_BLE_BASE + 0x00))
#define BT_BLE_CONF_REG         (*(volatile uint32_t *)(BT_BLE_BASE + 0x04))
#define BT_BLE_STA_REG          (*(volatile uint32_t *)(BT_BLE_BASE + 0x08))
#define BT_BLE_INT_STA_REG      (*(volatile uint32_t *)(BT_BLE_BASE + 0x0C))
#define BT_BLE_INT_ENA_REG      (*(volatile uint32_t *)(BT_BLE_BASE + 0x10))

#define BT_BLE_CTRL_EN          (1u << 0)
#define BT_BLE_CTRL_RESET       (1u << 1)

/* ------------------------------------------------------------------
 * Wi-Fi / Bluetooth coexistence
 * ------------------------------------------------------------------ */
#define BT_COEX_BASE            0x3FFE6000UL
#define BT_COEX_CTRL_REG        (*(volatile uint32_t *)(BT_COEX_BASE + 0x00))
#define BT_COEX_CONF_REG        (*(volatile uint32_t *)(BT_COEX_BASE + 0x04))

#define BT_COEX_MODE_BT_ONLY    0
#define BT_COEX_MODE_WIFI_ONLY  1
#define BT_COEX_MODE_BOTH       2

/* ------------------------------------------------------------------
 * HCI packet types
 * ------------------------------------------------------------------ */
#define HCI_CMD_PKT             0x01
#define HCI_ACL_DATA_PKT        0x02
#define HCI_SCO_DATA_PKT        0x03
#define HCI_EVENT_PKT           0x04
#define HCI_ISO_DATA_PKT        0x05

/* HCI command header (little-endian) */
typedef struct __attribute__((packed)) {
    uint16_t opcode;
    uint8_t  param_len;
} hci_cmd_hdr_t;

#define HCI_OPCODE(ogf, ocf)    ((uint16_t)(((ogf) << 10) | (ocf)))

/* HCI event header */
typedef struct __attribute__((packed)) {
    uint8_t  evt_code;
    uint8_t  param_len;
} hci_evt_hdr_t;

#define HCI_EVT_CMD_COMPLETE    0x0E
#define HCI_EVT_CMD_STATUS      0x0F
#define HCI_EVT_LE_META         0x3E

/* HCI Command Complete Event */
typedef struct __attribute__((packed)) {
    uint8_t  num_hci_cmd_pkts;
    uint16_t opcode;
    uint8_t  status;
} hci_evt_cmd_complete_t;

/* HCI LE Meta Event */
typedef struct __attribute__((packed)) {
    uint8_t  subevent;
    uint8_t  data[1];
} hci_evt_le_meta_t;

#define HCI_SUBEVT_LE_ADVERTISING_REPORT     0x02
#define HCI_SUBEVT_LE_CONNECTION_COMPLETE     0x01

/* ACL data header */
typedef struct __attribute__((packed)) {
    uint16_t handle_flags;
    uint16_t data_len;
} hci_acl_hdr_t;

/* ------------------------------------------------------------------
 * OGF (Opcode Group Field)
 * ------------------------------------------------------------------ */
#define OGF_LINK_CONTROL        0x01
#define OGF_LINK_POLICY         0x02
#define OGF_CONTROLLER_BB       0x03
#define OGF_INFO_PARAMS         0x04
#define OGF_STATUS_PARAMS       0x05
#define OGF_LE_CONTROL          0x08

/* ------------------------------------------------------------------
 * Common HCI commands
 * ------------------------------------------------------------------ */
#define OCF_RESET               0x0003
#define OCF_READ_BD_ADDR        0x0009
#define OCF_LE_SET_ADV_PARAMS   0x0006
#define OCF_LE_SET_ADV_DATA     0x0008
#define OCF_LE_SET_SCAN_RSP_DATA 0x0009
#define OCF_LE_SET_ADV_ENABLE   0x000A
#define OCF_LE_SET_SCAN_PARAMS  0x000B
#define OCF_LE_SET_SCAN_ENABLE  0x000C
#define OCF_LE_CREATE_CONN      0x000D

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */

int  bt_init(void);
void bt_deinit(void);

int  bt_hci_evt_available(void);
int  bt_hci_send_cmd_raw(const uint8_t *buf, uint32_t len);
int  bt_hci_recv_evt_raw(uint8_t *buf, uint32_t max_len, uint32_t timeout_ms);
int  bt_hci_send_acl_raw(const uint8_t *buf, uint32_t len);
int  bt_hci_recv_acl_raw(uint8_t *buf, uint32_t max_len, uint32_t timeout_ms);

int  bt_hci_cmd(const uint16_t opcode, const uint8_t *params, uint8_t param_len);
int  bt_hci_cmd_read_status(const uint16_t opcode, const uint8_t *params,
                            uint8_t param_len, uint8_t *status_out);
int  bt_hci_cmd_read_evt(const uint16_t opcode, const uint8_t *params,
                          uint8_t param_len, uint8_t *buf, uint32_t max_len,
                          uint32_t *out_len);

int  bt_reset(void);
int  bt_read_bd_addr(uint8_t addr[6]);
int  bt_le_set_advertise_enable(uint8_t enable);
int  bt_le_set_advertise_params(uint16_t interval_min, uint16_t interval_max,
                                uint8_t type, uint8_t own_addr_type,
                                uint8_t peer_addr_type, const uint8_t *peer_addr,
                                uint8_t channel_map, uint8_t filter_policy);
int  bt_le_set_advertise_data(const uint8_t *data, uint8_t len);
int  bt_le_set_scan_response_data(const uint8_t *data, uint8_t len);
int  bt_le_set_scan_params(uint8_t type, uint16_t interval, uint16_t window,
                           uint8_t own_addr_type, uint8_t filter_policy);
int  bt_le_set_scan_enable(uint8_t enable, uint8_t filter_duplicates);

#ifdef __cplusplus
}
#endif

#endif
