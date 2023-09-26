/** @file
 *  @brief Nordic NUS sample application (Nordic Uart Service)
 */

/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2020 Callender-Consulting LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/types.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

#include "nus.h"

#define LOG_LEVEL 3
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

/* AUTH_NUMERIC_COMPARISON result in in LESC Numeric Comparison authentication
 * Undefine this result in LESC Passkey Input
 */
#define AUTH_NUMERIC_COMPARISON

#define DEVICE_NAME         CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN     (sizeof(DEVICE_NAME) - 1)

struct bt_conn *default_conn;
    /** Level 0: Only for BR/EDR special cases, like SDP */
    /** Level 1: No encryption and no authentication. */
    /** Level 2: Encryption and no authentication (no MITM). */
    /** Level 3: Encryption and authentication (MITM). */
    /** Level 4: Authenticated Secure Connections and 128-bit key. */
#define BT_SECURITY     BT_SECURITY_L1
bt_security_t           g_level = BT_SECURITY_L1;

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, 0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E),
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection failed (err %u)", err);
    } 

    default_conn = bt_conn_ref(conn);
    LOG_INF("Connected");

    int ret = bt_conn_set_security(default_conn, BT_SECURITY);
    if (ret) {
        LOG_ERR("Failed to set security (err %d)", ret);
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected (reason %u)", reason);

    if (default_conn) {
        bt_conn_unref(default_conn);
        default_conn = NULL;
    }
}

#if defined(CONFIG_BT_SMP)
static void identity_resolved(struct bt_conn *conn, const bt_addr_le_t *rpa, const bt_addr_le_t *identity)
{
    char addr_identity[BT_ADDR_LE_STR_LEN];
    char addr_rpa[BT_ADDR_LE_STR_LEN];

    LOG_INF("%s", __func__);

    bt_addr_le_to_str(identity, addr_identity, sizeof(addr_identity));
    bt_addr_le_to_str(rpa, addr_rpa, sizeof(addr_rpa));

    LOG_INF("Identity resolved %s -> %s", addr_rpa, addr_identity);
}

static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    LOG_INF("%s", __func__);

    if (err) {
        LOG_ERR("%s failed: %d", __func__, err);
        return;
    }

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Security changed: %s level %u", addr, level);
    g_level = level;
}
#endif /* defined(CONFIG_BT_SMP) */

static struct bt_conn_cb conn_callbacks = {
    .connected          = connected,
    .disconnected       = disconnected,
    .le_param_req       = NULL,
    .le_param_updated   = NULL,
#if defined(CONFIG_BT_SMP)
    .identity_resolved  = identity_resolved,
    .security_changed   = security_changed
#endif /* defined(CONFIG_BT_SMP) */
};

static void nus_data_handler(ble_nus_data_evt_t * p_evt)
{
   LOG_INF("NUS data received, len: %d, data: %s\n", 
           p_evt->rx_data.length, p_evt->rx_data.p_data);
}

static void bt_ready(int err)
{
    ble_nus_init_t init = {
      .data_handler = nus_data_handler
    };
     
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return;
    }

    LOG_INF("Bluetooth initialized");

    err = nus_init(&init);
    if (err) {
        LOG_ERR("NUS failed to init (err %d)", err);
        return;
    }

    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        return;
    }

    LOG_INF("Advertising successfully started");
}

static void auth_cancel(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    bt_conn_auth_cancel(conn);
    LOG_INF("Pairing cancelled: %s", addr);
}

static void auth_pairing_confirm(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];
    int err;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Pairing Confirm for %s", addr);
    if (conn == default_conn) {
        err = bt_conn_auth_pairing_confirm(conn);
        if (err) {
            LOG_ERR("Confirm failed(err %d)", err);
        }
        else {
            LOG_INF("Confirmed!");
        }
    }
}

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Passkey Display for %s: %06u", addr, passkey);
}

#if defined(AUTH_NUMERIC_COMPARISON)
static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
    char addr[BT_ADDR_LE_STR_LEN];
    int err;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    LOG_INF("Passkey Confirm for %s: %06u", addr, passkey);
    if (conn == default_conn) {
        err = bt_conn_auth_passkey_confirm(conn);
        if (err) {
            LOG_ERR("Confirm failed(err %d)", err);
        }
        else {
            LOG_INF("Confirmed!");
        }
    }
}

/* result in DISPLAY_YESNO and Numeric Comparison
 * check bt_conn_get_io_capa() in subsys/bluetooth/host/conn.c
 */
static struct bt_conn_auth_cb auth_cb_display_yesno = {
    .passkey_display    = auth_passkey_display,
    .passkey_entry      = NULL,
    .passkey_confirm    = auth_passkey_confirm,
    .cancel             = auth_cancel,
    .pairing_confirm    = auth_pairing_confirm
};

#else
/* result in DISPLAY_ONLY and Passkey Input
 * check bt_conn_get_io_capa() in subsys/bluetooth/host/conn.c
 */
static struct bt_conn_auth_cb auth_cb_display_only = {
    .passkey_display    = auth_passkey_display,
    .passkey_entry      = NULL,
    .passkey_confirm    = NULL,
    .cancel             = auth_cancel,
    .pairing_confirm    = auth_pairing_confirm
};
#endif

void main(void)
{
    int err;
    char tx_char;
    int tx_index = 0;

    err = bt_enable(bt_ready);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return;
    }

    bt_conn_cb_register(&conn_callbacks);

#if defined(AUTH_NUMERIC_COMPARISON)
    bt_conn_auth_cb_register(&auth_cb_display_yesno);
#else
    bt_conn_auth_cb_register(&auth_cb_display_only);
#endif

    /* Implement notification. At the moment there is no suitable way
     * of starting delayed work so we do it here
     */
    while (1) {
        k_sleep(K_MSEC(1000));

        tx_char = 'A' + tx_index % 26;
        /* Send NUS notifications */
        if (g_level == BT_SECURITY && nus_notify(default_conn, tx_char) == 0)
        {
           LOG_INF("NUS sent %c", tx_char);
           tx_index++;
        }
    }
}
