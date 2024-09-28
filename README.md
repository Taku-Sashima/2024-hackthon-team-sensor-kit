# Sensor-Kit
## OpenHack U 2024 Tokyoに出場した際に作成したものです
その時の動画↓
https://www.youtube.com/live/VB_LScUyFnk?si=VjJbpBYXvLOy0Pyn[https://www.youtube.com/live/VB_LScUyFnk?si=VjJbpBYXvLOy0Pyn]

## M5StickCPlusからサーバーにかおりデータを送信するプログラム
### プログラムの流れ
- M5StickCPlusに接続したセンサーからかおりのデータを取得する
- 取得したデータを整形してJSONにする
- WIFI経由でサーバーにかおりのデータを送る

### 実装した機能
システム開発段階で使用する部分
- 見本となるデータを入れるためのデータ取得、そのデータをサーバーに送信する
- pythonにてm5から送られてきたデータ素取得しCSVファイルとして保存し、分析できる形にする

- ユーザーが使用する部分
- m5のボタンを押してかおりのデータを取得、整形、送信
- m5の画面位現在の状態を表示
