#include <PMserial.h> // Arduino library for PM sensors with serial interface
#include "Arduino_ST7789.h"
#include "Adafruit_GFX.h"
#include <WiFi.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include "Fonts/FreeSansBold18pt7b.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "Fonts/FreeSansBold9pt7b.h"

String hostname = "";
/* InfluxDB Connection Parameters */
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define INFLUXDB_URL ""
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN ""
// InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG ""
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET ""
#define TZ_INFO ""

/* InfluxDB Data Points */
Point PMS("PMS");

struct pms_struct
{
  uint16_t pm01 = 0;
  uint16_t pm25 = 0;
  uint16_t pm10 = 0;
  uint16_t n0p3 = 0;
  uint16_t n0p5 = 0;
  uint16_t n1p0 = 0;
  uint16_t n2p5 = 0;
  uint16_t n5p0 = 0;
  uint16_t n10p0 = 0;
  uint32_t ERR_COUNT = 0;
} pms_data;
constexpr auto PMS_RX = 3;
constexpr auto PMS_TX = 4;
Arduino_ST7789 tft = Arduino_ST7789(10, 5, 7, 6, 2);
SerialPM pms(PMS7003, PMS_RX, PMS_TX); // PMSx003, RX, TX
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
void setup()
{
  Serial.begin(115200);
  tft.init(240, 240);

  tft.fillScreen(BLUE);
  tft.fillRect(0, 126, 240, 114, BLACK);
  tft.setFont(&FreeSansBold9pt7b);
  tft.setCursor(1, 20);
  tft.print("Particulate Matter Readings");
  tft.setCursor(0, 140);
  tft.print("Dust Bin Count / m");
  tft.setCursor(162, 137);
  tft.print("3");
  tft.setFont(&FreeSansBold18pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(0, 55);
  tft.print("PM");
  tft.setCursor(0, 85);
  tft.print("PM");
  tft.setCursor(0, 115);
  tft.print("PM");
  tft.setFont(&FreeSansBold12pt7b);
  tft.setCursor(53, 58);
  tft.print("1");
  tft.setCursor(53, 85);
  tft.print("2.5");
  tft.setCursor(56, 118);
  tft.print("10");
  tft.setTextColor(GREEN);
  tft.setCursor(0, 238);
  tft.print("0.3");
  tft.setCursor(40, 238);
  tft.print("0.5");
  tft.setCursor(83, 238);
  tft.print("1.0");
  tft.setCursor(130, 238);
  tft.print("2.5");
  tft.setCursor(175, 238);
  tft.print("5");
  tft.setCursor(200, 238);
  tft.print("10");
  tft.setFont(&FreeSansBold18pt7b);
  tft.setTextColor(RED);
  tft.setCursor(120, 55);
  tft.print("...");
  tft.setTextColor(MAGENTA);
  tft.setCursor(120, 85);
  tft.print("...");
  tft.setTextColor(YELLOW);
  tft.setCursor(120, 115);
  tft.print("...");
  pms.init();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[INIT] ERROR CONNECTING TO WIFI");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(3500);
  }
  configTzTime("SGT-8", "pool.ntp.org", "time.nis.gov");
  client.setHTTPOptions(HTTPOptions().httpReadTimeout(200));
  client.setHTTPOptions(HTTPOptions().connectionReuse(true));
  // Check server connection
  if (client.validateConnection())
  {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  }
  else
  {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  Serial.println("[INIT] IFDB CONNECTION OK");
}

void loop()
{
  Serial.println("trs");
  pms.read();
  if (pms)
  {
    pms_data.pm01 = pms.pm01;
    pms_data.pm25 = pms.pm25;
    pms_data.pm10 = pms.pm10;
    pms_data.n0p3 = pms.n0p3;
    pms_data.n0p5 = pms.n0p5;
    pms_data.n1p0 = pms.n1p0;
    pms_data.n2p5 = pms.n2p5;
    pms_data.n5p0 = pms.n5p0;
    pms_data.n10p0 = pms.n10p0;
  }
  else
  {
  }

  PMS.clearFields();
  PMS.clearTags();
  PMS.addTag("UID", "N/A");
  PMS.addField("PM0.1", (pms_data.pm01));
  PMS.addField("PM2.5", (pms_data.pm25));
  PMS.addField("PM1.0", (pms_data.pm10));
  PMS.addField("N0.3", (pms_data.n0p3));
  PMS.addField("N0.5", (pms_data.n0p5));
  PMS.addField("N1.0", (pms_data.n1p0));
  PMS.addField("N2.5", (pms_data.n2p5));
  PMS.addField("N5.0", (pms_data.n5p0));
  PMS.addField("N10.0", (pms_data.n10p0));
  PMS.addField("Error Count", (pms_data.ERR_COUNT));
  if (!client.writePoint(PMS))
  {
    Serial.println("ERROR");
    pms_data.ERR_COUNT++;
  }
  tft.setFont(&FreeSansBold18pt7b);
  tft.fillRect(120, 25, 120, 30, BLUE);
  tft.setTextColor(RED);
  tft.setCursor(120, 55);
  tft.print(pms_data.pm01);
  tft.fillRect(120, 55, 120, 30, BLUE);
  tft.setTextColor(MAGENTA);
  tft.setCursor(120, 85);
  tft.print(pms_data.pm25);
  tft.fillRect(120, 85, 120, 33, BLUE);
  tft.setTextColor(YELLOW);
  tft.setCursor(120, 115);
  tft.print(pms_data.pm10);
  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(0xFC00);

  tft.fillRect(2, 150, 36, 70, 0x3838);
  tft.fillRect(2, 150, 36, (int16_t)((float)(10000 - pms_data.n0p3) * 0.007), BLACK);
  if ((int16_t)((float)(10000 - pms_data.n0p3) * 0.007) < 40)
  {
    tft.setCursor(2, 165 + (int16_t)((float)(10000 - pms_data.n0p3) * 0.007));
    tft.printf("%.1f", ((float)pms_data.n0p3 / 1000));
    tft.setCursor(27, 165 + (int16_t)((float)(10000 - pms_data.n0p3) * 0.007));
    tft.print("k");
  }
  else
  {
    tft.setCursor(2, 165 + (int16_t)40);
    tft.printf("%.1f", ((float)pms_data.n0p3 / 1000));
    tft.setCursor(27, 165 + (int16_t)40);
    tft.print("k");
  }

  tft.fillRect(42, 150, 36, 70, 0x3838);
  tft.fillRect(42, 150, 36, (int16_t)((float)(4000 - pms_data.n0p5) * 0.0175), BLACK);
  if ((int16_t)((float)(4000 - pms_data.n0p5) * 0.0175) < 40)
  {
    tft.setCursor(42, 165 + (int16_t)((float)(4000 - pms_data.n0p5) * 0.0175));
    tft.printf("%.1f", ((float)pms_data.n0p5 / 1000));
    tft.setCursor(67, 165 + (int16_t)((float)(4000 - pms_data.n0p5) * 0.0175));
    tft.print("k");
  }
  else
  {
    tft.setCursor(42, 165 + (int16_t)40);
    tft.printf("%.1f", ((float)pms_data.n0p5 / 1000));
    tft.setCursor(67, 165 + (int16_t)40);
    tft.print("k");
  }

  tft.fillRect(82, 150, 36, 70, 0x3838);
  tft.fillRect(82, 150, 36, (int16_t)((float)(700 - pms_data.n1p0) * 0.1), BLACK);
  if ((int16_t)((float)(700 - pms_data.n1p0) * 0.1) < 40)
  {

    tft.setCursor(82, 165 + (int16_t)((float)(700 - pms_data.n1p0) * 0.1));
    tft.print(pms_data.n1p0);
  }
  else
  {

    tft.setCursor(82, 165 + (int16_t)40);
    tft.print(pms_data.n1p0);
  }

  tft.fillRect(122, 150, 36, 70, 0x3838);
  tft.fillRect(122, 150, 36, (int16_t)((float)(100 - pms_data.n2p5) * 0.7), BLACK);
  if ((int16_t)((float)(100 - pms_data.n2p5) * 0.7) < 40)
  {

    tft.setCursor(122, 165 + (int16_t)((float)(100 - pms_data.n2p5) * 0.7));
    tft.print(pms_data.n2p5);
  }
  else
  {

    tft.setCursor(122, 165 + (int16_t)40);
    tft.print(pms_data.n2p5);
  }

  tft.fillRect(162, 150, 36, 70, 0x3838);
  tft.fillRect(162, 150, 36, (int16_t)((float)(20 - pms_data.n5p0) * 3.5), BLACK);
  if ((int16_t)((float)(20 - pms_data.n5p0) * 3.5) < 40)
  {
    tft.setCursor(162, 165 + (int16_t)((float)(20 - pms_data.n5p0) * 3.5));
    tft.print(pms_data.n5p0);
  }
  else
  {
    tft.setCursor(162, 165 + (int16_t)40);
    tft.print(pms_data.n5p0);
  }

  tft.fillRect(202, 150, 36, 70, 0x3838);
  tft.fillRect(202, 150, 36, (int16_t)((float)(10 - pms_data.n10p0) * 7), BLACK);
  if ((int16_t)((float)(10 - pms_data.n10p0) * 7) < 40)
  {
    tft.setCursor(202, 165 + (int16_t)((float)(10 - pms_data.n10p0) * 7));
    tft.print(pms_data.n10p0);
  }
  else
  {
    tft.setCursor(202, 165 + (int16_t)40);
    tft.print(pms_data.n10p0);
  }

  Serial.print("[IFDB] TXN ");
  delay(3500);
}