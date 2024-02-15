import sys
import serial
import datetime
import csv

#接続されているシリアルポートに書き換える必要があります。
#arduino側でserialmonitorを使ってると動きません。
#CSVのファイル名とitem_idを都度変更する

count =0
dataAmount = 310
#都度変更する部分
item_id = 6
file_name = './testes.csv'



ser = serial.Serial('/dev/cu.usbserial-7952B9681E',115200) #ポートの情報を記入
#都度変更する部分
with open(file_name, 'w') as f:
    print("item_id,date,id,temperature,humidity,pressure,gas_value,diff_gas_value",file=f)

while(count <dataAmount):
    id, _, temperature, humidity, pressure, gas_value, diff_gas_value, _ = ser.readline().decode('utf-8').split(',')
    count += 1
    date = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    print(item_id,date, id, temperature, humidity, pressure, gas_value, diff_gas_value, count)
    #都度変更する部分
    with open(file_name, 'a') as f:
        print(f"{item_id},{date},{id},{temperature},{humidity},{pressure},{gas_value},{diff_gas_value}",file=f)

print("finifh")