

#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"

#include "time.h"
#include "sys/time.h"

const char * const storage_namespace = "storage";

struct packet {
  
};

// Store min, max, average over 5 minute span.
// Store 
float angle;
float angle_max, angle_min, angle_average.

struct reading {
};


// Write 4096 bytes. Single page.
// 32+32 header
// 126 * 32 entries
// With a perfect blob entry
//  32 bit NS, Type, Span, ChunkIdx, CRC, Key, Data

// Read blob from storage.
esp_err_t data_read(const char *key, byte* &buffer, size_t &readBytes) {
  nvs_handle_t handle;
  esp_err_t err;

  assert(buffer == NULL);
  readBytes = 0;
  
  err = nvs_open(storage_namespace, NVS_READWRITE, &handle);
  if(err != ESP_OK) return err;

  size_t bytes = 0;
  err = nvs_get_blob(handle, key, NULL, &bytes);
  if(err != ESP_OK) goto clean_release;

  if(bytes == 0) {
    // Nothing saved
    
    err = ESP_OK;
    goto clean_release;
  }

  byte *buf = malloc(bytes);
  if(buf == null) {
    // No memory
    err = ESP_ERR_NO_MEM;
    goto clean_release;
  }
  
  err = nvs_get_blob(handle, key, buf, &bytes);
  if(err != ESP_OK) goto clean_free;

  
  // Do something with data.
  buffer = buf;
  readBytes = bytes;
  
  buf = NULL;

  err = ESP_OK;
clean_free:
  // Free Allocated Data
  if(buf) free(buf);
  
clean_release:
  nvs_close(handle);
  return err;
}

esp_err_t data_write(const char * key, byte* &buffer, size_t bytes) {
  nvs_handle_t handle;
  esp_err_t err;

  if(buffer == NULL) {
    return ERR_OK;
  }
  
  err = nvs_open(storage_namespace, NVS_READWRITE, &handle);
  if(err != ESP_OK) return err;

  size_t bytes = 0;
  err = nvs_set_blob(handle, key, buffer, bytes);
  switch(err) {
    case ESP_ERR_NVS_NOT_FOUND:
      goto clean_release;
    case ESP_OK:
      break;
    default:
      goto clean_release;
  }

  // Commit
  err = nvs_commit(handle);
  if(err != ESP_OK) {
    goto clean_release;
  }

  err = ESP_OK;
  
clean_release:
  nvs_close(handle);
  return err;
}

// Generate Key
//  15 chars.
// (3 - record)(date 12 yyyyMMddHHmm)
// (1 - record)(date 14 yyyyMMddHHmmss)
// ddd202002064400
int make_key(const char key[16], time_t rawtime, char ident) {
  tm gmtime(ts);
  snprintf(key, sizeof(key), "")

  struct tm *info = NULL;
  info = gmtime(rawtime);

  strftime(key, 15, "_%Y%m%d%H%M%S", info);
  key[0] = ident;
  return 0;
}
int now_key() {
  time_t rawtime;
  time(&rawtime);

  char key[16];
  make_key(key, rawtime, 'A');

}

// Info + data



void data_init()
{
   esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    
}
