#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiAP.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiGeneric.h>
#include <WiFiSTA.h>
#include <esp_http_server.h>
#include <nvs_flash.h>
#include "esp_netif.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include <stdlib.h>
#include <sys/param.h>
#include <string.h>
#include <unistd.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"



/*************************/
/*      PROTOTYPES       */
/*************************/
///////////////////////////
httpd_handle_t start_webserver(void);
esp_err_t get_handler(httpd_req_t *req);
esp_err_t post_wifi_provisioning_handler(httpd_req_t *req);
bool initializeNVS(void);
String displayStartPage();


/*************************/
/*      VARIABLES        */
/*************************/
///////////////////////////
const char *ssid = "MyESP32AP";        // Replace with your desired Access Point name
const char *password = "yourpassword"; // Optional password for the AP

const char *NVS_WIFI_AP = "nvs_wifi_ap";
const char *NVS_WIFI_CLIENT = "nvs_wifi_client";
const char *NVS_HTTP_SERVER = "nvs_http_server";

nvs_handle_t nvsHandle;

WiFiServer server(80); // Create a TCP server on port 80
WiFiClient client;



/*************************/
/*     PRIMARY CODE      */
/*************************/
///////////////////////////
void setup()
{
  Serial.begin(115200);

  if(initializeNVS()) {
    // Configure the ESP32 as an access point
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.println("Starting HTTP Server...");
    Serial.println("AP IP address: ");
    Serial.println(IP);
    start_webserver();
  }

}

void loop() {}



/*************************/
/*    WEBSERVER CODE     */
/*************************/
///////////////////////////
/* URI handler structure for GET /uri */
httpd_uri_t uri_get = {
  .uri = "/",
  .method = HTTP_GET,
  .handler = get_handler,
  .user_ctx = NULL
};


/* URI handler structure for POST /uri */
static httpd_uri_t uri_post_wifi_provisioning = {
  .uri = "/wifi_provisioning",
  .method = HTTP_POST,
  .handler = post_wifi_provisioning_handler,
  .user_ctx = NULL
};


/* Function for starting the webserver */
httpd_handle_t start_webserver(void)
{
  /* Generate default configuration */
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  /* Empty handle to esp_http_server */
  httpd_handle_t server = NULL;

  /* Start the httpd server */
  if (httpd_start(&server, &config) == ESP_OK)
  {
    /* Register URI handlers */
    httpd_register_uri_handler(server, &uri_get);
    httpd_register_uri_handler(server, &uri_post_wifi_provisioning);

    Serial.println("HTTP Server started!");
  }
  /* If server failed to start, handle will be NULL */
  return server;
}


/* Function for stopping the webserver */
void stop_webserver(httpd_handle_t server)
{
  if (server) {
    httpd_stop(server);
    Serial.println("HTTP Server Stopped...");
  }
}


/* Our URI handler function to be called during GET / request */
esp_err_t get_handler(httpd_req_t *req)
{
  /* Send a simple response */
  const char resp[] = "URI GET Response";
  httpd_resp_send(req, displayStartPage().c_str(), HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}


/* Our URI handler function to be called during POST /uri request */
esp_err_t post_wifi_provisioning_handler(httpd_req_t *req)
{
  /* Destination buffer for content of HTTP POST request.
   * httpd_req_recv() accepts char* only, but content could
   * as well be any binary data (needs type casting).
   * In case of string data, null will be absent, and
   * content length would give length of string */
  char content[100];

  /* Truncate if content length larger than the buffer */
  size_t recv_size = MIN(req->content_len, sizeof(content));

  int ret = httpd_req_recv(req, content, recv_size);
  if (ret <= 0)
  { /* 0 return value indicates connection closed */
    /* Check if timeout occurred */
    if (ret == HTTPD_SOCK_ERR_TIMEOUT)
    {
      /* In case of timeout one can choose to retry calling
       * httpd_req_recv(), but to keep it simple, here we
       * respond with an HTTP 408 (Request Timeout) error */
      httpd_resp_send_408(req);
    }
    /* In case of error, returning ESP_FAIL will
     * ensure that the underlying socket is closed */
    return ESP_FAIL;
  }

  /* Send a simple response */
  const char resp[] = "URI POST Response";
  httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}


/*************************/
/*         NVS           */
/*************************/
///////////////////////////
// Returns tue if already initialized before, else false //
bool initializeNVS(void)
{
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();

    return false;
  }

  return true;
}


String displayStartPage() {
  String content;
  content = "<!DOCTYPE html> <html>  <head><meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>  ";
  content += " <title>ESP WiFi Manager</title>   <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
  content += " body{margin-top: 50px;} h1 {color: #444444;margin: 30px auto 30px;}   h3 {color: #ffffff;margin-bottom: 50px;}   label{display:inline-block;width: 160px;text-align: center;font-size: 1.5rem; font-weight: bold;color: #ffffff;}";
  content += " form{margin: 0 auto; width: 360px; padding: 1em; border: 1px solid #CCC; border-radius: 1em; background-color: #077c9f;}    input {margin: 0.5em;font-size: 1.5rem; }  "; 
  content += "   .styled {    border: 0;    line-height: 2.5;    padding: 0 20px;    font-size: 1.5rem;    text-align: center;    color: #fff;    text-shadow: 1px 1px 1px #000; ";
  content += "  border-radius: 10px;    background-color: rgba(220, 0, 0, 1);    background-image: linear-gradient(to top left,  rgba(0, 0, 0, .2), rgba(0, 0, 0, .2) 30%, ";
  content += "   rgba(0, 0, 0, 0));    box-shadow: inset 2px 2px 3px rgba(255, 255, 255, .6),   inset -2px -2px 3px rgba(0, 0, 0, .6);.styled:hover {   background-color: rgba(255, 0, 0, 1);} ";
  content += ".styled:active {    box-shadow: inset -2px -2px 3px rgba(255, 255, 255, .6), inset 2px 2px 3px rgba(0, 0, 0, .6);}  </style>  "; 
  content += "  <meta charset='UTF-8'>    </head>   <body>   <h1>Welcome to WiFi Update</h1>    <h1>ESP32C3 6 Channel ADC</h1>";  
  content += "       <h1>Enter local WiFi SSID&PSW</h1>    <form method='post' action='/wifi_provisioning'> <div><label>WiFi SSID</label> <input name='ssid' length=32></div> ";  
  content += "   <div><label>WiFi Password </label> <input name='pass'  length=32></div>  "; 
  content += " <h3>Please Double Check Before Saving</h3>   <button class= 'favorite styled ' type= 'submit'>SAVE</button> </form></body></html>  ";

  return content;  
}


/*
  err = nvs_open(storageNamespace, NVS_READWRITE, &nvsHandle);

  // int32_t valueGet;
  // err = nvs_get_i32(nvsHandle, "restart_counter", &valueGet);

  // err = nvs_set_i32(nvsHandle, "restart_counter", 32);
  // err = nvs_commit(nvsHandle);

  // Close
  // nvs_close(my_handle);
*/