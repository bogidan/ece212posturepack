
#include "arduino.h"
#include <ctype.h>
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

#include "core.h"
#include "ringbuf.h"
#include "_main.h"

static const char* TAG = "BlueTooth";

const char * _bt_name = "Posture Pack Custom BT";

int checkEspErr(esp_err_t err, const char *context = "")
{
  switch(err) {
    case ESP_OK: return 0;
    case ESP_FAIL:
      ESP_LOGI(TAG, "%s ERR_FAIL", context) ;
      return -1;
    default: {
      ESP_LOGE(TAG, "%s", esp_err_to_name(err));
      return -1;
    }
  }
}


// Data
static uint32_t _bt_handle = 0;
static ringbuf  _bt_ring(12);

// Functions
static void _bt_gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
static void _bt_spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);



static const char *const lookup_esp_spp_status_t(esp_spp_status_t status)
{
  #define X(_Value) \
    case _Value: return __STRINGIFY(_Value);
  switch(status) {
    default: return "{UNHANDLED::esp_spp_status_t}";
    X(ESP_BT_STATUS_SUCCESS)
    X(ESP_BT_STATUS_FAIL)
    X(ESP_BT_STATUS_NOT_READY)
    X(ESP_BT_STATUS_NOMEM)
    X(ESP_BT_STATUS_BUSY)
    X(ESP_BT_STATUS_DONE)
    X(ESP_BT_STATUS_UNSUPPORTED)
    X(ESP_BT_STATUS_PARM_INVALID)
    X(ESP_BT_STATUS_UNHANDLED)
    X(ESP_BT_STATUS_AUTH_FAILURE)
    X(ESP_BT_STATUS_RMT_DEV_DOWN)
    X(ESP_BT_STATUS_AUTH_REJECTED)
    X(ESP_BT_STATUS_INVALID_STATIC_RAND_ADDR)
    X(ESP_BT_STATUS_PENDING)
    X(ESP_BT_STATUS_UNACCEPT_CONN_INTERVAL)
    X(ESP_BT_STATUS_PARAM_OUT_OF_RANGE)
    X(ESP_BT_STATUS_TIMEOUT)
    X(ESP_BT_STATUS_PEER_LE_DATA_LEN_UNSUPPORTED)
    X(ESP_BT_STATUS_CONTROL_LE_DATA_LEN_UNSUPPORTED)
    X(ESP_BT_STATUS_ERR_ILLEGAL_PARAMETER_FMT)
    X(ESP_BT_STATUS_MEMORY_FULL)
    //X(ESP_BT_STATUS_EIR_TOO_LARGE)
  }
  #undef X
}


static void _bt_gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
  esp_err_t err;
  ESP_LOGV(TAG, "ESP_BT_GAP_EVENT %02X", event);
  
  switch (event) {
    case ESP_BT_GAP_AUTH_CMPL_EVT:{
      if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
        ESP_LOGI(TAG, "authentication success: %s", param->auth_cmpl.device_name);
        esp_log_buffer_hex(TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
      } else {
        ESP_LOGE(TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
      }
      break;
    }
    case ESP_BT_GAP_PIN_REQ_EVT:{
      ESP_LOGI(TAG, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
      if (param->pin_req.min_16_digit) {
        ESP_LOGI(TAG, "Input pin code: 0000 0000 0000 0000");
        esp_bt_pin_code_t pin_code = {0};
        esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
      } else {
        ESP_LOGI(TAG, "Input pin code: 1234");
        esp_bt_pin_code_t pin_code;
        pin_code[0] = '1';
        pin_code[1] = '2';
        pin_code[2] = '3';
        pin_code[3] = '4';
        esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
      }
      break;
    }
    case ESP_BT_GAP_CFM_REQ_EVT:
      ESP_LOGI(TAG, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d", param->cfm_req.num_val);
      esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
      break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
      ESP_LOGI(TAG, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%d", param->key_notif.passkey);
      break;
    case ESP_BT_GAP_KEY_REQ_EVT:
      ESP_LOGI(TAG, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
      break;
    default:
      ESP_LOGI(TAG, "ESP_BT_GAP_***_EVT %02X Unhandled", event);
      break;
  }
}

static void _bt_spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  esp_err_t err;
  ESP_LOGV(TAG, "ESP_SPP_EVENT %02X", event);
  
  switch(event) {
    case ESP_SPP_INIT_EVT:
      ESP_LOGI(TAG, "ESP_SPP_INIT_EVT");
      err = esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
      err = esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, "SPP_SERVER");
      if(checkEspErr(err, "ssp_start_srv")) {
        break;
      }
      break;

    //case ESP_SSP_DISCOVERY_COMP_EVT:
    //  ESP_LOGI(TAG, "ESP_SSP_DISCOVERY_COMP_EVT");
    //  break;

    case ESP_SPP_OPEN_EVT:
      ESP_LOGI(TAG, "ESP_SPP_OPEN_EVT");
      break;
          
    case ESP_SPP_CLOSE_EVT:
      ESP_LOGI(TAG, "ESP_SPP_CLOSE_EVT");
      lookup_esp_spp_status_t(param->close.status);
      _bt_handle = 0;
      // Disconnect
      break;
      
    case ESP_SPP_START_EVT:
      ESP_LOGI(TAG, "ESP_SPP_START_EVT");
      break;

    case ESP_SPP_CL_INIT_EVT:
      ESP_LOGI(TAG, "ESP_SPP_CL_INIT_EVT");
      break;
      
    case ESP_SPP_DATA_IND_EVT:// Data received
      ESP_LOGI(TAG, "ESP_SPP_DATA_IND_EVT len=%d handle=%d", param->data_ind.len, param->data_ind.handle);
      //esp_log_buffer_hexdump (TAG, (param->data_ind.data, param->data_ind.len));
      ESP_LOG_BUFFER_CHAR_LEVEL(TAG, (param->data_ind.data), (param->data_ind.len), ESP_LOG_INFO);
      
      //Serial.write(param->data_ind.data, param->data_ind.len);
      //Serial.write('\n');

      if (param->data_ind.len <= 0) {
        ESP_LOGE(TAG, "Invalid data length");
        break;
      }
      
      ESP_LOGI(TAG, "Data Recv: %s", (param->data_ind.data));
      {
        int recved =  param->data_ind.len;
        int saved = _bt_ring.push(param->data_ind.data, param->data_ind.len);
        ESP_LOGV(TAG, "recieved: %d, saved: %d", recved, saved);
      }
      break;

    case ESP_SPP_CONG_EVT:
      ESP_LOGI(TAG, "ESP_SPP_CONG_EVT");
      break;

    case ESP_SPP_WRITE_EVT:
      ESP_LOGI(TAG, "ESP_SPP_WRITE_EVT");
      break;
  
    case ESP_SPP_SRV_OPEN_EVT:
      ESP_LOGI(TAG, "ESP_SPP_SRV_OPEN_EVT %2X {%s}", param->open.status, lookup_esp_spp_status_t(param->open.status));
      _bt_handle = param->open.handle;
      // Connect
      break;

    default:
      ESP_LOGE(TAG, "ESP_SPP_***_EVT %02X Unhandled", event);
      break;
  }
}



// API Implementation

size_t _bt_write(const void* buf, size_t bytes) {
  if(_bt_handle == NULL) return 0;
  
  esp_err_t err = esp_spp_write(_bt_handle, bytes, (uint8_t *)buf);
  if(checkEspErr(err, "esp_spp_write")) {
    return 0;
  }

  return bytes;
}

size_t _bt_read (void* buf, size_t bytes) {
  return _bt_ring.pull(buf, bytes);
}

size_t _bt_avail() {
  return _bt_ring.avail();
}

int _bt_init() {
  if (!btStarted() && !btStart()) {
    _println("Failed to start Bluetooth");
    return -1;
  }

  esp_err_t err = 0;
  esp_bluedroid_status_t bt_state = esp_bluedroid_get_status();
  
  if (bt_state == ESP_BLUEDROID_STATUS_UNINITIALIZED) {
    err = esp_bluedroid_init();
    if (checkEspErr(err, "bluedroid_init")) {
      return 1;
    }

    err = esp_bluedroid_enable();
    if (checkEspErr(err, "bluedroid_init")) {
      return 1;
    }

    err = esp_spp_register_callback(_bt_spp_callback);
    if (checkEspErr(err, "esp_spp_register_callback")) {
      return 1;
    }

    err = esp_spp_init(ESP_SPP_MODE_CB);
    if (checkEspErr(err, "esp_spp_init")) {
      return 1;
    }

    err = esp_bredr_tx_power_set(ESP_PWR_LVL_N2, ESP_PWR_LVL_P7);
    if (checkEspErr(err, "tx_power_set")) {
      return 1;
    }

    err = esp_bt_dev_set_device_name(_bt_name);
    if (checkEspErr(err, "set_device_name")) {
      return 1;
    }

    return 0;
  }
  
  return 0;
}
