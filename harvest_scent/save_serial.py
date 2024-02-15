import sys
import serial
import datetime
import csv

#接続されているシリアルポートに書き換える必要があります。
#arduino側でserialmonitorを使ってると動きません。
#CSVのファイル名とitem_idを都度変更する

count =0
dataAmount = 10
start_fetch_data = False
index = 0
#都度変更する部分
item_id = 0
file_name = './test444.csv'

def get_data():
	global count
	while(count <dataAmount):
		id, _, temperature, humidity, pressure, gas_value, diff_gas_value, _ = ser.readline().decode('utf-8').split(',')
		count += 1
		date = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')

		print(item_id,date, id, temperature, humidity, pressure, gas_value, diff_gas_value, count)
		with open(file_name, 'a') as f:
			print(f"{item_id},{date},{id},{temperature},{humidity},{pressure},{gas_value},{diff_gas_value}",file=f)


ser = serial.Serial('/dev/cu.usbserial-7952B9681E',115200) #ポートの情報を記入

with open(file_name, 'w') as f:
	print("item_id,date,id,temperature,humidity,pressure,gas_value,diff_gas_value",file=f)

while(1):
	index, _, temperature, humidity, pressure, gas_value, diff_gas_value, _ = ser.readline().decode('utf-8').split(',')
	print("index is", index)
	if int(index)==9:
		print("work")
		get_data()
		break
	else:
		print(index)

print("finifh")