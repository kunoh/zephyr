#include <zephyr/settings/settings.h>
#include <stddef.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/types.h>
#include <tm_hello_module.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/bluetooth/services/hrs.h>
#include <zephyr/bluetooth/uuid.h>
#include <event_mgr.h>
#include <zephyr/logging/log.h>
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)



void put_event(struct k_timer *dummy)
{

  static int counter = 1;
  struct event event = {
     .type = EVENT_ERROR,
     .value = counter++
  };
  event_put(&event);
}

K_TIMER_DEFINE(event_timer, put_event , NULL);
LOG_MODULE_REGISTER(main);
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_BAS_VAL), BT_UUID_16_ENCODE(BT_UUID_HRS_VAL))};

static void connected(struct bt_conn *conn, uint8_t err) {
  if (err) {
    printk("Connection failed: %u\n", err);
    return;
  }

  char addr[BT_ADDR_LE_STR_LEN];
  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
  printk("Connection established!		\n\
	Connected to: %s					\n",
         addr);
}

static void disconnected(struct bt_conn *conn, uint8_t reason) {
  printk("Disconnected (reason %u)\n", reason);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

static void auth_cancel(struct bt_conn *conn) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  printk("Pairing cancelled: %s\n", addr);
}

void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx) {
  printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}

static struct bt_gatt_cb gatt_callbacks = {.att_mtu_updated = mtu_updated};

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
  printk("Passkey for %s: %06u\n", addr, passkey);
}

static struct bt_conn_auth_cb auth_cb_display = {
    .passkey_display = auth_passkey_display,
    .passkey_entry = NULL,
    .cancel = auth_cancel,
};

static void bt_ready(int err) {
  char addr_s[BT_ADDR_LE_STR_LEN];
  bt_addr_le_t addr = {0};
  size_t count = 1;

  if (err) {
    printk("Bluetooth init failed (err %d)\n", err);
    return;
  }

  printk("Bluetooth initialized\n");
  /* Start advertising */

  if (IS_ENABLED(CONFIG_SETTINGS)) {
    settings_load();
  }
    err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), sd,
                          ARRAY_SIZE(sd));
  if (err) {
    printk("Advertising failed to start (err %d)\n", err);
  }


  bt_id_get(&addr, &count);
  bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));

  printk("Beacon started, advertising as %s\n", addr_s);
}

static void bas_notify(void) {
  uint8_t battery_level = bt_bas_get_battery_level();

  battery_level--;

  if (!battery_level) {
    battery_level = 100U;
  }

  bt_bas_set_battery_level(battery_level);
}


static void hrs_notify(void)
{
    static uint8_t heartrate = 90U;

    /* Heartrate measurements simulation */
    heartrate++;
    if (heartrate == 160U) {
        heartrate = 90U;
    }

    bt_hrs_notify(heartrate);
}


void main(void) {
  int err;
  struct event event;
  print_hello();
  printk("BLE MCU started \n");

  /* Initialize the Bluetooth Subsystem */
  err = bt_enable(bt_ready);
  if (err) {
    printk("Bluetooth init failed (err %d)\n", err);
    return;
  }
  bt_gatt_cb_register(&gatt_callbacks);
  LOG_INF("GATT callbacek registered");
  bt_conn_auth_cb_register(&auth_cb_display);
  LOG_INF("BT conn atuth callback registered");

  while (1) {
    k_sleep(K_SECONDS(3));

    /* Battery level simulation */
    bas_notify();
    hrs_notify();
    event_get(&event);
    LOG_INF("Event error count %d", event.value);
  }
}
