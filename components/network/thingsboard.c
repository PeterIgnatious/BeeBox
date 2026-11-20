
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"

#include "thingsboard.h"
#include "thingsboard_config.h"

static const char *TAG = "THINGSBOARD";

esp_err_t thingsboard_send_telemetry(
    float temperature_dht22,
    float humidity,
    float weight,
    float temperature_ds18b20
)
{
    char url[256];
    char payload[256];

    snprintf(
        url,
        sizeof(url),
        "https://%s:%d/api/v1/%s/telemetry",
        THINGSBOARD_HOST,
        THINGSBOARD_PORT,
        THINGSBOARD_TOKEN
    );

    snprintf(
        payload,
        sizeof(payload),
        "{\"temperature\":%.2f,"
        "\"humidity\":%.2f,"
        "\"weight\":%.2f,"
        "\"temperature_ds18b20\":%.2f}",
        temperature_dht22,
        humidity,
        weight,
        temperature_ds18b20
    );

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000,
        .crt_bundle_attach = esp_crt_bundle_attach
    };

    esp_http_client_handle_t client =
        esp_http_client_init(&config);

    if (client == NULL)
    {
        ESP_LOGE(TAG, "Falha ao inicializar cliente HTTP");
        return ESP_FAIL;
    }

    esp_http_client_set_header(
        client,
        "Content-Type",
        "application/json"
    );

    esp_http_client_set_post_field(
        client,
        payload,
        strlen(payload)
    );

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int status_code =
            esp_http_client_get_status_code(client);

        ESP_LOGI(TAG, "HTTP Status = %d", status_code);

        if (status_code >= 200 && status_code < 300)
        {
            ESP_LOGI(TAG, "Telemetria enviada com sucesso!");
        }
        else
        {
            ESP_LOGE(TAG, "Servidor recusou a telemetria");
            err = ESP_FAIL;
        }
    }
    else
    {
        ESP_LOGE(
            TAG,
            "Erro no envio HTTPS: %s",
            esp_err_to_name(err)
        );
    }

    esp_http_client_cleanup(client);

    return err;
}