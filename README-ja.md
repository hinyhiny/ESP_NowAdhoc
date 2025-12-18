[[In English README.md] ](https://github.com/cpei2025/ESPNowAdhoc/blob/main/README.md)

![MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Arduino](https://img.shields.io/badge/Arduino-Compatible-blue.svg)
![ESP32](https://img.shields.io/badge/ESP32-Compatible-green.svg)
![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange.svg)

# ESP_NowAdhoc Library
ESP32用の高度なESP-NOWアドホックネットワークライブラリ。Wi-Fiアクセスポイントなしで、複数のESP32デバイス間の直接通信を実現します。
ESP_NOWを利用したアドホックネットワーク環境を簡単に安全に構築できるパッケージライブラリ。アドホックネットワークとして利用するのに必要な、グループ設定、ロール設定、セキュリティー設定、ブロードキャストによる自動ピアリング、ハートビートによる接続済みピアの状態監視を、パッケージ化。３ステップで、簡単にアドホックネットワーク環境をESP32だけで実現。これまで高額で、複雑だった、ESP32環境での無線通信を簡単に低価格で実現可能。

## 特徴
- 複雑なESP_NOWライブラリクラスを利用した開発不要で、アドホックネットワークが簡単に構築可能
- UUIDグループ管理（接続のためのアドバタイジングパケットのグループUUIDと、ユニキャスト通信のグループUUIDを分離し、他のESP_NOWとの混信を強力に防御）
- ロール設定可能（サーバー、クライアントを１パラメーターで設定可能。様々なアドホックネットワーク形態を簡単に構築可能）
- 複雑なコーディング不要で、双方向通信可能
- 複雑なコーディング不要で、複数台オートピアリング（ESP_NOWの仕様で暗号化なし時、最大20ピアまで）
- クライアント、サーバーも、パラメーター変更するだけのワンソースマルチユース
- 簡単な暗号化通信設定
- 接続ピア状態を表示するファンクション
- 実データ領域の設定が可能（必要メモリーの最適化が可能）
- 高セキュリテイー（ユニキャスト通信時、グループUUIDを独立しているので暗号化通信下でのグループ UUID露出なし）
- ハートビートによる状態監視（ハートビートで接続ピアの状態監視。接続が切れた場合も、相手がが復旧すれば自動接続。）
- ESP32内蔵Wi-Fi機能を利用しているので、別途通信モジュールが不要
- ESP_NOW V2の環境の場合1470byteまで送信可能(1470byteまで送信可能かどうかは、起動時にシリアルで確認してください)

## 応用例
- リモコンシステム
- 双方向通信を必要とする、低価格なリモートコントロールシステム
- 双方向通信を必要とする、複数箇所でコントロールできるリモートコントロールシステム
- 自動接続・Wi-FI AP不要を活かした自立型集団ドローン・ロボット管理
- 自動接続・Wi-FI AP不要を活かした野外での自立型センサーネットワーク
- 自動接続・Wi-FI AP不要を活かした自立型災害救援システム

## ESPNowAdhocペイロード（送信パケット）仕様
- グループUUID（char 37 Bytes）
- ロール（サーバー、クライアント）（bool 1 byte）
- Wi-Fiチャンネル（将来自動チャンネル変更機能実装のために保留）（int）
- セキュリテイーモードの有無（bool 1 byte）
- コマンド（レジスト、ハートビート、データ）（int）
- 実データ(ESPNOW_DATA_SIZEで可変設定可能、ただしESP_NOWの最大送信送信バイトを超えないこと)

## オプションで設定可能なパラメータ
- ブロードキャスト間隔設定
- ハートビート送信間隔設定
- ハートビートタイムアウト設定
- 実データのデータサイズ設定
- ピア登録、ハートビート状態を確認できるデバックモード設定

## 対応機種・利用に際して
- ESP_NOWが利用できる環境
-- ESP32-Sシリーズ: ESP32-S2、ESP32-S3なども対応。
-- ESP32-Cシリーズ: ESP32-C3なども対応。
-- M5Stackシリーズ: ESP32マイコン搭載のため、ESP-NOWが利用可能。 
- クラアント同士はピアリングしないため、最低１台サーバーロール設定の機器が必要
- ESP_NOW v2が利用できる環境の場合、
- 本ライブラリ利用に関するお問い合わせは、　GITにて受け付け

## 定数・デフォルト値
> これらはヘッダ内のプリプロセッサ定義で上書き可能です。

- **ESPNOW_WIFI_CHANNEL**: デフォルト `4`  
- **ADV_GROUP_ID**: デフォルト `"906b868f-7e9b-4c21-b587-70c8d5fadfee"`
<br>**注意:**セキュリティーの観点からメインプログラム上で変更してください
- **GROUP_ID**: デフォルト `"73f8e3bb-aab2-4808-8efe-c061c88e48c2"`
<br>**注意:**セキュリティーの観点からメインプログラム上で変更してください  
- **HEARTBEAT_TIMEOUT**: デフォルト `5000` ms  
- **HEARTBEAT_INTERVAL**: デフォルト `1000` ms  
- **BROADCAST_INTERVAL**: デフォルト `1000` ms  
- **ESPNOW_DATA_SIZE**: デフォルト `1000` bytes ※
<br>**注意:** ライブラリ内部の確保サイズですが、ESP-NOW の実際の送信上限（デバイス依存、一般的に ~250 バイト程度）を超えないようにしてください。大きなデータは分割送信が必要です。

## 導入方法

## STEP1 ライブラリインストール

### Arduino IDE
1. Sketch → Include Library → Manage Libraries...
2. "ESPNowAdhoc"を検索
3. インストールをクリック

### 手動インストール
1. [Releases](https://github.com/cpei2025/ESPNowAdhoc/releases)からZIPをダウンロード
2. Arduino IDE: Sketch → Include Library → Add .ZIP Library...
3. **ESP32 ボードパッケージ** がインストール済みであることを確認してください。

### PlatformIO
platformio.iniファイルに以下を追加：
```
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    https://github.com/cpei2025/ESPNowAdhoc.git
```

## STEP2 初期設定(重要)
1. ブロードキャストと、ユニキャスト用のグループUUID（必須）
```
#define ADV_GROUP_ID "906b868f-7e9b-4c21-b587-70c8d5fadfee"
#define GROUP_ID "73f8e3bb-aab2-4808-8efe-c061c88e48c2"
```
2. 暗号化通信の有無の設定（必須）と、暗号化の場合に必要な公開鍵、秘密鍵を設定
```
#define SECURITY true//（必須）
#define PMK_STRING "hogehoge54321"//公開鍵16文字以内（セキュリティーモードの場合は必須）
#define LMK_STRING "hogehoge12345"//秘密鍵16文字以内（セキュリティーモードの場合は必須）
```
3. ロールの設定
```
#define ROLE true// ロール設定 (true: サーバー, false: クライアント)（必須）
```
## STEP3 コーディング・リリース

1. `ESPNowAdhoc` インスタンス生成  
2. `begin()` で初期化（Server/Client、セキュリティ有無を指定）  
3. `setDataCallback()` / `setPeerEventCallback()` を設定  
4. `loop()` 内で `update()` を毎回呼ぶ  
5. 必要に応じ `sendToAll()` / `sendToServer()` / `sendToClients()` を呼ぶ


## よくある問題と解決策

### 問題1: ピアが接続されない
1. 異なるチャンネルを使用している
2. グループIDが一致していない
3. セキュリティ設定が異なる

### 問題2: データ送信に失敗する
1. ペイロードのデーテフィールド容量（bytes）がESPNOW_DATA_SIZEを超えている
2. 物理的な距離が遠すぎる
3. #define ESPNOW_DATA_SIZE を設定の場合、 #include "ESPNowAdhoc.h"のの前に記述していること
4. デバックモードで確認（setup()内で）
```
espnow.setDebug(false);
```
を行い、相手と、ピアリングができているか、ハートビート信号を相手から受信できているか確認してください。 

## よくある利用パターン（チェックリスト）
- `begin()` で正しい **role (Server/Client)** を渡す  
- セキュリティ有効時は適切な **PMK/LMK の長さと管理** を行う  
- `loop()` 内で **`update()` を定期実行**  
- デバッグを有効にして接続ログを確認（`setDebug(true)`）  
- 大容量データは **分割送信** を実装する  
- グループ分離が必要なら **`setGroupID()`** で UUID を異なるものに設定する

## FAQ
### Q:最大で何台のデバイスを接続できますか？
#### A: セキュリティモードによって異なります：
- セキュリティ無効: 最大19ピア
- セキュリティ有効: 最大5ピア


### Q:通信距離はどのくらいですか？
#### A: 環境によって異なります：
- 屋内（見通し）: 10-30m
- 屋外（開けた場所）: 100-200m
壁などの障害物がある場合: 通信距離が大幅に減少します。


### Q:データ転送速度はどのくらいですか？
#### A: 理論上の最大速度：
- 1対1通信: 約1Mbps
- 実際のスループット: 500-800kbps
- レイテンシ: 通常10-50ms
壁などの障害物がある場合: 通信距離が大幅に減少します。


### Q:異なるESP32モデルで互換性はありますか？
#### A: 可能:
異なるモデルでも、ESP_NOW対応機種間であれば、送受信可能です


## ライセンス・サポート
- ライセンスはリポジトリ内の `LICENSE` ファイルを参照してください。  
- バグ報告・機能要望は GitHub Issues に上げてください。

---

## APIに関しては、WIKIを参照
[[APIリファレンス]](https://github.com/cpei2025/ESPNowAdhoc/wiki/API%E3%83%AA%E3%83%95%E3%82%A1%E3%83%AC%E3%83%B3%E3%82%B9)



