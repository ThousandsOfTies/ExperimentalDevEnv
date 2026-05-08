# アーキテクチャ コンセプト

本PoC環境は、**「組み込みソフトウェア開発から『実機依存』と『環境構築の煩雑さ』を排除する」**ことを目的としています。

これを実現するために、以下の3つのコア・アーキテクチャを採用しています。

---

## 1. クラウドIDEとクロスコンパイル (GitHub Codespaces)

ローカルPCのOSや環境に依存せず、ブラウザまたはVSCode互換エディタ（Antigravity等）から直接 GitHub Codespaces に接続します。

* **メリット**: 開発環境のセットアップが数秒で完了し、チーム全員が完全に同一のツールチェーン（`aarch64-linux-gnu-gcc` 等）を使用できます。

---

## 2. クラウド・ハードウェア・シミュレーション (AWS EC2 Graviton)

実機（Raspberry Pi 5など）と同じ **ARM64 (aarch64)** アーキテクチャである AWS EC2 Graviton インスタンスを利用します。

* **完全なバイナリ互換**: EC2上でクロスコンパイルされたバイナリを動かし、それが正常に動作すれば、実機でもそのまま動きます。
* **CUSEスタブと LD_PRELOAD**: 物理的なGPIOピンやI2Cデバイスが存在しないクラウド上で、`CUSE`（Character device in Userspace）や共有ライブラリのフックを用いてハードウェアをエミュレートします。
* **Virtual Hardware Panel**: エミュレートされたハードウェアの挙動（LEDの点灯、ボタンの押下、センサー値の変化）は、WebSocket経由でブラウザ上のパネルに同期され、視覚的にテストが可能です。

---

## 3. SSH/scp によるデプロイ

シンプルかつ確実な SSH/scp でファイル転送とシェルアクセスを統一します。

* **EC2（シミュレータ）**: `make deploy EC2=vibecode-graviton` で scp 転送、`ssh vibecode-graviton` でシェルアクセス。
* **RasPi5（実機）**: `make deploy EC2=pi@raspberrypi KEY=~/.ssh/raspi.pem` で scp 転送、`ssh pi@raspberrypi` でシェルアクセス。

**結果として、ターゲットが EC2 か RasPi5 かを問わず、同じ `make deploy` コマンドでデプロイできます。**

```bash
# EC2 へ
make deploy EC2=vibecode-graviton

# RasPi5 へ
make deploy EC2=pi@raspberrypi KEY=~/.ssh/raspi.pem
```
