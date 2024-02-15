// M5Atom で BME688 のガスセンサ・ヒーターを操作するサンプル
// 2022/1/6 @ksasao
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
#include <M5StickCPlus.h>
#include <HTTPClient.h>
#include <math.h>
#include "bme68xLibrary.h"
Bme68x bme;

#include <M5GFX.h>// M5GFXライブラリのインクルード
M5GFX lcd; 
M5Canvas canvas(&lcd);


//Wifi系
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUDP.h>
//同じ階層に　WifiSecret.h　ファイルを作る



#define NEW_GAS_MEAS (BME68X_GASM_VALID_MSK | BME68X_HEAT_STAB_MSK | BME68X_NEW_DATA_MSK)
#define MEAS_DUR 140

// // Atomic proto
// #define SDA_PIN 32
// #define SCL_PIN 33

// Grove
#define SDA_PIN 32
#define SCL_PIN 33

#define BME688_I2C_ADDR 0x76

bool doGetData =false;

float last = 0;


void harvest_data(void){
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
        //M5の画面に計測中の文字を表示
        float current = log(data.gas_resistance); // 値の変動が大きいので対数をとるといい感じです

        Serial.print(String(data.gas_index)+",");
        Serial.print(String(millis()) + ",");
        Serial.print(String(data.temperature) + ","); // 周囲の温度湿度も結構影響があります
        Serial.print(String(data.humidity) + ",");
        Serial.print(String(data.pressure) + ",");        
        Serial.print(String(current,3)+",");
        Serial.print(String(current-last+10,3)+",");  // ガスの脱着は温度変化に敏感なので差分もつかうと良いです
        Serial.println("");


        last = current;
        delay(20);
      }
    } while (nFieldsLeft);
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
  M5.Lcd.setTextSize(3);
  M5.IMU.Init();

  lcd.begin();
  canvas.createSprite(lcd.width(), lcd.height()/2);
  canvas.setTextColor(WHITE); 
  canvas.setFont(&fonts::lgfxJapanGothic_40);
  
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
  //M5の中央ボタンを押したらtrueになる
  if ( M5.BtnA.wasPressed() ) {
    doGetData = M5.BtnA.wasPressed();
  }else{
    M5.update();
  }

  canvas.fillRect(0, 0, lcd.width(), 120, BLACK);
  if (doGetData){
    harvest_data();
    
    canvas.println("データ取得中");
  }else{
    canvas.println("待機中");
  }

  
  canvas.setCursor(0, 0);
  canvas.pushSprite(&lcd, 0, lcd.height()/2);

}
