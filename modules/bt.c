#include <stdint.h>
#include "bt.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

/* ------------------------------------------------------------------
 * Internal helpers
 * ------------------------------------------------------------------ */

static inline uint32_t read_ccount(void)
{
    uint32_t val;
    __asm__ volatile("rsr.ccount %0" : "=r"(val));
    return val;
}

static void udelay(uint32_t us)
{
    uint32_t start = read_ccount();
    uint32_t ticks = us * 240;
    while ((read_ccount() - start) < ticks) {}
}

/* ------------------------------------------------------------------
 * BT controller power-up sequence
 * ------------------------------------------------------------------ */

int bt_init(void)
{
    /* 1. Enable Bluetooth clocks via DPORT */
    uint32_t clk_en = DPORT_PERIP_CLK_EN_REG;
    clk_en |= DPORT_BT_CLK_EN | DPORT_BLE_CLK_EN;
    DPORT_PERIP_CLK_EN_REG = clk_en;

    udelay(10);

    /* 2. Release Bluetooth from reset */
    uint32_t rst_en = DPORT_PERIP_RST_EN_REG;
    rst_en &= ~(DPORT_BT_RST | DPORT_BLE_RST);
    DPORT_PERIP_RST_EN_REG = rst_en;

    udelay(10);

    /* 3. Reset BT LC (Link Controller) */
    BT_LC_CTRL_REG |= BT_LC_CTRL_RESET;
    udelay(1);
    BT_LC_CTRL_REG &= ~BT_LC_CTRL_RESET;
    udelay(1);
    BT_LC_CTRL_REG |= BT_LC_CTRL_ENABLE;

    /* 4. Power up baseband */
    BT_BB_PWR_CTRL_REG |= BT_BB_PWR_UP | BT_BB_PWR_BT | BT_BB_PWR_BLE;
    udelay(5);

    /* 5. Power up RF */
    BT_RF_PWR_CTRL_REG |= BT_RF_PWR_UP;
    udelay(5);

    /* 6. Reset and enable HCI interface */
    BT_HCI_CTRL_REG |= BT_HCI_CTRL_RESET;
    udelay(1);
    BT_HCI_CTRL_REG &= ~BT_HCI_CTRL_RESET;
    udelay(1);
    BT_HCI_CTRL_REG |= BT_HCI_CTRL_EN;

    /* 7. Enable HCI interrupts */
    BT_HCI_INT_ENA_REG = BT_HCI_INT_CMD | BT_HCI_INT_EVT
                       | BT_HCI_INT_DATA_OUT | BT_HCI_INT_DATA_IN;

    /* 8. Enable BLE core */
    BT_BLE_CTRL_REG |= BT_BLE_CTRL_RESET;
    udelay(1);
    BT_BLE_CTRL_REG &= ~BT_BLE_CTRL_RESET;
    udelay(1);
    BT_BLE_CTRL_REG |= BT_BLE_CTRL_EN;

    return 0;
}

void bt_deinit(void)
{
    BT_BLE_CTRL_REG &= ~BT_BLE_CTRL_EN;
    BT_HCI_CTRL_REG &= ~BT_HCI_CTRL_EN;
    BT_RF_PWR_CTRL_REG &= ~BT_RF_PWR_UP;
    BT_BB_PWR_CTRL_REG &= ~(BT_BB_PWR_UP | BT_BB_PWR_BT | BT_BB_PWR_BLE);
    BT_LC_CTRL_REG &= ~BT_LC_CTRL_ENABLE;

    uint32_t rst_en = DPORT_PERIP_RST_EN_REG;
    rst_en |= DPORT_BT_RST | DPORT_BLE_RST;
    DPORT_PERIP_RST_EN_REG = rst_en;

    uint32_t clk_en = DPORT_PERIP_CLK_EN_REG;
    clk_en &= ~(DPORT_BT_CLK_EN | DPORT_BLE_CLK_EN);
    DPORT_PERIP_CLK_EN_REG = clk_en;
}

/* ------------------------------------------------------------------
 * HCI transport via register interface
 * ------------------------------------------------------------------ */

int bt_hci_evt_available(void)
{
    return (BT_HCI_FIFO_STA_REG & BT_HCI_FIFO_EVT_RDY) ? 1 : 0;
}

int bt_hci_send_cmd_raw(const uint8_t *buf, uint32_t len)
{
    uint32_t i;

    if (!buf || len < 3 || len > 256)
        return -1;

    /* Wait for command FIFO ready */
    uint32_t timeout = 100000;
    while (!(BT_HCI_FIFO_STA_REG & BT_HCI_FIFO_CMD_RDY)) {
        if (--timeout == 0)
            return -2;
    }

    /* Write HCI command type */
    BT_HCI_CMD_REG = (uint32_t)HCI_CMD_PKT;

    /* Write command header: opcode (2 bytes) + param_len (1 byte) */
    BT_HCI_CMD_REG = (uint32_t)buf[0] | ((uint32_t)buf[1] << 8);
    BT_HCI_CMD_REG = (uint32_t)buf[2];

    /* Write parameters */
    for (i = 3; i < len; i++) {
        BT_HCI_CMD_REG = (uint32_t)buf[i];
    }

    return (int)len;
}

int bt_hci_recv_evt_raw(uint8_t *buf, uint32_t max_len, uint32_t timeout_ms)
{
    if (!buf || max_len < 2)
        return -1;

    {
        uint32_t timeout_us = timeout_ms * 1000;
        uint32_t start = read_ccount();

        while (!(BT_HCI_FIFO_STA_REG & BT_HCI_FIFO_EVT_RDY)) {
            if (timeout_ms && ((read_ccount() - start) / 240) >= timeout_us)
                return -2;
        }
    }

    /* Read and discard event type (should be HCI_EVENT_PKT) */
    BT_HCI_EVT_REG;

    /* Read event header: evt_code + param_len */
    buf[0] = (uint8_t)BT_HCI_EVT_REG;
    buf[1] = (uint8_t)BT_HCI_EVT_REG;

    uint32_t total_len = buf[1] + 2;

    if (total_len > max_len)
        total_len = max_len;

    /* Read event parameters */
    for (uint32_t i = 2; i < total_len; i++) {
        buf[i] = (uint8_t)BT_HCI_EVT_REG;
    }

    return (int)total_len;
}

int bt_hci_send_acl_raw(const uint8_t *buf, uint32_t len)
{
    uint32_t i;

    if (!buf || len < 4)
        return -1;

    uint32_t timeout = 100000;
    while (!(BT_HCI_FIFO_STA_REG & BT_HCI_FIFO_DATA_IN_RDY)) {
        if (--timeout == 0)
            return -2;
    }

    BT_HCI_DATA_IN_REG = (uint32_t)HCI_ACL_DATA_PKT;

    for (i = 0; i < len; i++) {
        BT_HCI_DATA_IN_REG = (uint32_t)buf[i];
    }

    return (int)len;
}

int bt_hci_recv_acl_raw(uint8_t *buf, uint32_t max_len, uint32_t timeout_ms)
{
    if (!buf || max_len < 4)
        return -1;

    {
        uint32_t timeout_us = timeout_ms * 1000;
        uint32_t start = read_ccount();

        while (!(BT_HCI_FIFO_STA_REG & BT_HCI_FIFO_DATA_IN_RDY)) {
            if (timeout_ms && ((read_ccount() - start) / 240) >= timeout_us)
                return -2;
        }
    }

    uint32_t pkt_type = BT_HCI_DATA_OUT_REG;
    (void)pkt_type;

    for (uint32_t i = 0; i < max_len; i++) {
        buf[i] = (uint8_t)BT_HCI_DATA_OUT_REG;
    }

    return (int)max_len;
}

/* ------------------------------------------------------------------
 * Higher-level HCI helpers
 * ------------------------------------------------------------------ */

int bt_hci_cmd(const uint16_t opcode, const uint8_t *params, uint8_t param_len)
{
    uint8_t buf[256];
    uint32_t i;

    buf[0] = (uint8_t)(opcode & 0xFF);
    buf[1] = (uint8_t)((opcode >> 8) & 0xFF);
    buf[2] = param_len;

    for (i = 0; i < param_len; i++) {
        buf[3 + i] = params[i];
    }

    return bt_hci_send_cmd_raw(buf, 3 + param_len);
}

int bt_hci_cmd_read_status(const uint16_t opcode, const uint8_t *params,
                           uint8_t param_len, uint8_t *status_out)
{
    uint8_t evt_buf[32];
    int ret;

    ret = bt_hci_cmd(opcode, params, param_len);
    if (ret < 0)
        return ret;

    ret = bt_hci_recv_evt_raw(evt_buf, sizeof(evt_buf), 5000);
    if (ret < 0)
        return ret;

    if (evt_buf[0] == HCI_EVT_CMD_STATUS) {
        if (status_out)
            *status_out = evt_buf[3];
        return evt_buf[3];
    }

    if (evt_buf[0] == HCI_EVT_CMD_COMPLETE) {
        hci_evt_cmd_complete_t *cc = (hci_evt_cmd_complete_t *)&evt_buf[2];
        if (cc->opcode == opcode) {
            if (status_out)
                *status_out = cc->status;
            return cc->status;
        }
    }

    return -3;
}

int bt_hci_cmd_read_evt(const uint16_t opcode, const uint8_t *params,
                        uint8_t param_len, uint8_t *buf, uint32_t max_len,
                        uint32_t *out_len)
{
    uint8_t evt_buf[64];
    int ret;

    ret = bt_hci_cmd(opcode, params, param_len);
    if (ret < 0)
        return ret;

    ret = bt_hci_recv_evt_raw(evt_buf, sizeof(evt_buf), 5000);
    if (ret < 0)
        return ret;

    if (evt_buf[0] == HCI_EVT_CMD_COMPLETE) {
        hci_evt_cmd_complete_t *cc = (hci_evt_cmd_complete_t *)&evt_buf[2];
        uint32_t cc_param_len = evt_buf[1];
        uint32_t copy_len = cc_param_len - 3;

        if (cc->opcode == opcode) {
            uint8_t *src = evt_buf + 6;
            if (copy_len > max_len)
                copy_len = max_len;
            for (uint32_t i = 0; i < copy_len; i++)
                buf[i] = src[i];
            if (out_len)
                *out_len = copy_len;
            return cc->status;
        }
    }

    return -3;
}

/* ------------------------------------------------------------------
 * Standard HCI commands
 * ------------------------------------------------------------------ */

int bt_reset(void)
{
    uint8_t status;
    int ret = bt_hci_cmd_read_status(HCI_OPCODE(OGF_CONTROLLER_BB, OCF_RESET),
                                      NULL, 0, &status);
    if (ret < 0)
        return ret;
    return (status == 0) ? 0 : -1;
}

int bt_read_bd_addr(uint8_t addr[6])
{
    uint8_t buf[8];
    uint32_t out_len;
    int ret = bt_hci_cmd_read_evt(
        HCI_OPCODE(OGF_INFO_PARAMS, OCF_READ_BD_ADDR),
        NULL, 0, buf, sizeof(buf), &out_len);
    if (ret < 0)
        return ret;
    if (ret != 0 || out_len < 6)
        return -1;
    for (int i = 0; i < 6; i++)
        addr[i] = buf[i];
    return 0;
}

int bt_le_set_advertise_enable(uint8_t enable)
{
    uint8_t status;
    uint8_t param = enable ? 1 : 0;
    int ret = bt_hci_cmd_read_status(
        HCI_OPCODE(OGF_LE_CONTROL, OCF_LE_SET_ADV_ENABLE),
        &param, 1, &status);
    if (ret < 0)
        return ret;
    return (status == 0) ? 0 : -1;
}

int bt_le_set_advertise_params(uint16_t interval_min, uint16_t interval_max,
                               uint8_t type, uint8_t own_addr_type,
                               uint8_t peer_addr_type, const uint8_t *peer_addr,
                               uint8_t channel_map, uint8_t filter_policy)
{
    uint8_t params[15];
    uint8_t status;
    int i;

    params[0] = (uint8_t)(interval_min & 0xFF);
    params[1] = (uint8_t)((interval_min >> 8) & 0xFF);
    params[2] = (uint8_t)(interval_max & 0xFF);
    params[3] = (uint8_t)((interval_max >> 8) & 0xFF);
    params[4] = type;
    params[5] = own_addr_type;
    params[6] = peer_addr_type;

    for (i = 0; i < 6; i++)
        params[7 + i] = peer_addr ? peer_addr[i] : 0;

    params[13] = channel_map;
    params[14] = filter_policy;

    int ret = bt_hci_cmd_read_status(
        HCI_OPCODE(OGF_LE_CONTROL, OCF_LE_SET_ADV_PARAMS),
        params, 15, &status);
    if (ret < 0)
        return ret;
    return (status == 0) ? 0 : -1;
}

int bt_le_set_advertise_data(const uint8_t *data, uint8_t len)
{
    uint8_t status;
    uint8_t params[32];

    if (len > 31)
        len = 31;

    params[0] = len;
    for (uint8_t i = 0; i < len; i++)
        params[1 + i] = data[i];

    int ret = bt_hci_cmd_read_status(
        HCI_OPCODE(OGF_LE_CONTROL, OCF_LE_SET_ADV_DATA),
        params, 1 + len, &status);
    if (ret < 0)
        return ret;
    return (status == 0) ? 0 : -1;
}

int bt_le_set_scan_response_data(const uint8_t *data, uint8_t len)
{
    uint8_t status;
    uint8_t params[32];

    if (len > 31)
        len = 31;

    params[0] = len;
    for (uint8_t i = 0; i < len; i++)
        params[1 + i] = data[i];

    int ret = bt_hci_cmd_read_status(
        HCI_OPCODE(OGF_LE_CONTROL, OCF_LE_SET_SCAN_RSP_DATA),
        params, 1 + len, &status);
    if (ret < 0)
        return ret;
    return (status == 0) ? 0 : -1;
}

int bt_le_set_scan_params(uint8_t type, uint16_t interval, uint16_t window,
                          uint8_t own_addr_type, uint8_t filter_policy)
{
    uint8_t params[7];
    uint8_t status;

    params[0] = type;
    params[1] = (uint8_t)(interval & 0xFF);
    params[2] = (uint8_t)((interval >> 8) & 0xFF);
    params[3] = (uint8_t)(window & 0xFF);
    params[4] = (uint8_t)((window >> 8) & 0xFF);
    params[5] = own_addr_type;
    params[6] = filter_policy;

    int ret = bt_hci_cmd_read_status(
        HCI_OPCODE(OGF_LE_CONTROL, OCF_LE_SET_SCAN_PARAMS),
        params, 7, &status);
    if (ret < 0)
        return ret;
    return (status == 0) ? 0 : -1;
}

int bt_le_set_scan_enable(uint8_t enable, uint8_t filter_duplicates)
{
    uint8_t params[2];
    uint8_t status;

    params[0] = enable ? 1 : 0;
    params[1] = filter_duplicates ? 1 : 0;

    int ret = bt_hci_cmd_read_status(
        HCI_OPCODE(OGF_LE_CONTROL, OCF_LE_SET_SCAN_ENABLE),
        params, 2, &status);
    if (ret < 0)
        return ret;
    return (status == 0) ? 0 : -1;
}
