
// 2024/1/12 @Taku-Sashima
// 
// 利用デバイス:
// デバイスは、下記などで入手してください
// BME688搭載 4種空気質センサモジュール（ガス/温度/気圧/湿度）
// https://www.switch-science.com/catalog/7383/
// 
// ライブラリの追加:
// https://github.com/BoschSensortec/Bosch-BME68x-Library
// で、Code > Download ZIP から ZIPファイルとしてダウンロードし、
// Arduino IDE の スケッチ > ライブラリをインクルード > .ZIP形式のライブラリをインストール
// からZIPファイルを登録してください。
//
// 参考:
//   https://twitter.com/ksasao/status/1479108937861709825


//M5とsensorのラオブラリ
// #include "M5Atom.h"
#include <Arduino.h>
#include <M5StickCPlus.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  

#include "bme68xLibrary.h"

//Wifi系
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUDP.h>
//同じ階層に　WifiSecret.h　ファイルを作る
#include "WifiSecret.h"

HTTPClient httpClient;
WiFiMulti wifiMulti;


const char *ssid = ssid_secret; //WiFIのSSIDを入力
const char *pass = pass_secret; // WiFiのパスワードを入力

const char *pc_addr = pc_addr_secret;  
const char *pc_port = pc_port_secret; //送信先のポート
// const int my_port = 50008;  //M5自身のポート


#define NEW_GAS_MEAS (BME68X_GASM_VALID_MSK | BME68X_HEAT_STAB_MSK | BME68X_NEW_DATA_MSK)
#define MEAS_DUR 140

// // Atomic proto
// #define SDA_PIN 32
// #define SCL_PIN 33

//Grove
#define SDA_PIN 32
#define SCL_PIN 33

#define BME688_I2C_ADDR 0x76

#include <math.h>
Bme68x bme;

float last = 0;
int count = 0;

StaticJsonDocument<2048> smel_json;

/**
 * @brief 取得したデータの一時格納場所、2次元配列
 */
char data_list[7][120];
// char gas_index[120];
// char millis_data[120];
// char temperature_data[120];
// char humidity_data[120];
// char pressure_data[120];
// char gas_value[120];
// char diff_gas_value[120];

// センサーデータの型に合わせた変数を定義する
int gas_index[120];
unsigned long millis_data[120];
float temperature_data[120];
float humidity_data[120];
float pressure_data[120];
float gas_value[120];
float diff_gas_value[120];



/**
 * @brief データをとって、2次元配列に入れる関数
 */
void get_data()
{
  bme68xData data;
  uint8_t nFieldsLeft = 0;
  
  /* data being fetched for every 140ms */
  delay(MEAS_DUR);

  if (bme.fetchData())
  {
    do
    {
      nFieldsLeft = bme.getData(data);
      if (data.status == NEW_GAS_MEAS)
      {
        float current = log(data.gas_resistance); // 値の変動が大きいので対数をとるといい感じです

        // Serial.print(String(data.gas_index)+",");
        // Serial.print(String(millis()) + ",");
        // Serial.print(String(data.temperature) + ","); // 周囲の温度湿度も結構影響があります
        // Serial.print(String(data.humidity) + ",");
        // Serial.print(String(data.pressure) + ",");        
        // Serial.print(String(current,3)+",");
        // Serial.print(String(current-last+10,3)+",");  // ガスの脱着は温度変化に敏感なので差分もつかうと良いです
        // Serial.println("");

        gas_index[count]=data.gas_index;
        millis_data[count]=millis();
        temperature_data[count]=data.temperature;
        humidity_data[count]=data.humidity;
        pressure_data[count]=data.pressure;
        gas_value[count]=(current,3);
        diff_gas_value[count]=(current-last+10,3);

        // Serial.print(count);
        Serial.print(String(gas_index[count]));
        Serial.print(String(millis_data[count]));
        Serial.print(String(temperature_data[count]));
        Serial.print(String(humidity_data[count]));
        Serial.print(String(pressure_data[count]));
        Serial.print(String(gas_value[count]));
        Serial.println(String(diff_gas_value[count]));
        count ++;

        last = current;
        delay(20);
      }
    } while (nFieldsLeft);
  }
}


/**
 * @brief JAONを作る関数
 */
void create_json()
{
  int first_index = 0;

  for(int i = 0; i < 15; i++){
    if(gas_index[i] == 0){
      first_index = i;
      break;
    }else{
      continue;
    }
  }

  char* colum[7] = {"id","date","temperature","humidity","pressure","gas_value","diff_gas_value"};

/**
* @brief jason配列を作る
*/
  for(int i=0; i<7; i++){
    JsonArray data = smel_json.createNestedArray(colum[i]);
  }

  for(int i=0; i<100; i++){
    int k = first_index + i;
    smel_json[0].add(String(gas_index[k]));
  }
  for(int i=0; i<100; i++){
    int k = first_index + i;
    smel_json[1].add(String(millis_data[k]));
  }
  for(int i=0; i<100; i++){
    int k = first_index + i;
    smel_json[2].add(String(temperature_data[k]));
  }
  for(int i=0; i<100; i++){
    int k = first_index + i;
    smel_json[3].add(String(humidity_data[k]));
  }
  for(int i=0; i<100; i++){
    int k = first_index + i;
    smel_json[4].add(String(pressure_data[k]));
  }
  for(int i=0; i<100; i++){
    int k = first_index + i;
    smel_json[5].add(String(gas_value[k]));
  }
  for(int i=0; i<100; i++){
    int k = first_index + i;
    smel_json[6].add(String(diff_gas_value[k]));
  }

}






/**
 * @brief Initializes the sensor and hardware settings
 */
void setup(void)
{
  Serial.begin(115200);
  M5.begin();
  delay(50);

  M5.Axp.ScreenBreath(34);
  M5.Lcd.setTextSize(2);
  M5.IMU.Init();

  //wifiに接続
  wifiMulti.addAP(ssid, pass);
  M5.Lcd.print("Waiting for WiFi to");
  M5.Lcd.print(ssid); 
  while(wifiMulti.run() != WL_CONNECTED) {
    M5.Lcd.print("."); 
  }

  //ディスプレイに表示
  M5.Lcd.println("WiFi connected");
  M5.Lcd.print("IP address = ");
  M5.Lcd.println(WiFi.localIP());
    
  
  Wire.begin(SDA_PIN, SCL_PIN);

  while (!Serial)
  {
    delay(10);
  }

  /* initializes the sensor based on I2C library */
  bme.begin(BME688_I2C_ADDR, Wire);

  if(bme.checkStatus())
  {
    if (bme.checkStatus() == BME68X_ERROR)
    {
      Serial.println("Sensor error:" + bme.statusString());
      return;
    }
    else if (bme.checkStatus() == BME68X_WARNING)
    {
      Serial.println("Sensor Warning:" + bme.statusString());
    }
  }
  
  /* Set the default configuration for temperature, pressure and humidity */
  bme.setTPH();

  /* ヒーターの温度(℃)の１サイクル分の温度変化。 200-400℃程度を指定。配列の長さは最大10。*/
  uint16_t tempProf[10] = { 320, 100, 100, 100, 200, 200, 200, 320, 320,320 };
  /* ヒーターの温度を保持する時間の割合。数値×MEAS_DUR(ms)保持される。保持時間は1～4032ms。指定温度に達するまで20-30ms程度が必要。 */
  uint16_t mulProf[10] = { 5, 2, 10, 30, 5, 5, 5, 5, 5, 5 };
  /* 各測定(温度,湿度,気圧,抵抗値)の繰り返し間隔(MEAS_DUR)から測定にかかる正味時間を引いたものをsharedHeatrDurに設定 */
  uint16_t sharedHeatrDur = MEAS_DUR - (bme.getMeasDur(BME68X_PARALLEL_MODE) / 1000);

  bme.setHeaterProf(tempProf, mulProf, sharedHeatrDur, 10);
  bme.setOpMode(BME68X_PARALLEL_MODE);

}






void loop(void)
{
  //ボタンを押したら起動する,contが115になったらfalsez→リセット
  if (true){
    get_data();
  }

  if (count == 115){
    create_json();

    if (wifiMulti.run() == WL_CONNECTED){

      char url[1024] = {0};
      sprintf(url, "http://%s:%s/m5_data_post",pc_addr_secret,pc_port);

      httpClient.begin(url);
      httpClient.addHeader("Content-Type", "application/json; charset=ascii");


      char output[2048]; 
      serializeJson(smel_json, output);

      int httpCode = httpClient.POST(output);

      if (httpCode == 200) {
          String response = httpClient.getString();
          Serial.printf("[HTTP RESPONSE]: %s", response);
      } else {
          Serial.printf("[HTTP ERR CODE]: %d", httpCode);
          String response = httpClient.getString();
          Serial.printf("[HTTP RESPONSE]: %s\n", response);
      }
      httpClient.end();
    }
    delay(5000);

  }
  

  

}


