# ExperimentalDevEnv

組み込みソフトウェア開発の「完全クラウド化」と「シームレスな実機デプロイ」を実現するためのPoC（概念実証）サンドボックスです。

このリポジトリは、**「実機がなくてもクラウド上でハードウェア込みのテストができ、実機が手に入った際も同じバイナリをそのまま流し込める」**という開発体験を目指して設計され、**EC2 シミュレーション・RasPi5 実機の両方で同一バイナリの動作を実証済み**です。

## ドキュメント ナビゲーション

### 📖 [1. アーキテクチャコンセプト (01_ARCHITECTURE.md)](docs/01_ARCHITECTURE.md)
* なぜ「クラウド化」なのか？
* Codespaces (ビルド) + EC2 Graviton (シミュレーション) + SSH/scp (デプロイ) の全体像と設計思想。

### 🔄 [2. 開発ワークフローとシーケンス (02_WORKFLOW.md)](docs/02_WORKFLOW.md)
* 開発者がコードを書いてから、シミュレータや実機で動かすまでの具体的なシーケンス図。
* コマンドリファレンス。

### 🛠️ [3. ハードウェアシミュレーション設定 (03_SIMULATION_SETUP.md)](docs/03_SIMULATION_SETUP.md)
* `cuse-stubs` を用いた I2C/GPIO/SPI スタブの仕組み。
* EC2上での Web ブリッジ起動方法と、Antigravity（ブラウザ）での Virtual Hardware Panel の使い方。

### 🔌 [4. 実機の配線 (04_HARDWARE_WIRING.md)](docs/04_HARDWARE_WIRING.md)
* RasPi5 とブレッドボードを使った LED / ボタン / I2C / SPI モジュールの配線図。

### 🎯 [5. PoC 成果まとめ (05_RESULTS.md)](docs/05_RESULTS.md)
* EC2 上でのフルシミュレーション動作確認結果と RasPi5 実機での動作確認結果。
* 実装したコンポーネントと得られた知見。

### 🌍 [6. 業界動向と本PoCの技術的価値 (06_INDUSTRY_TRENDS.md)](docs/06_INDUSTRY_TRENDS.md)
* Software Defined Vehicle (SDV) 等の最新トレンドとの比較。
* クラウドネイティブ組み込み開発における本アプローチの優位性と意義。

---

## クイックスタート概要

### EC2（シミュレーション）

```powershell
# Windows
.\ec2.ps1 start                                 # EC2 起動
```
```bash
# Codespaces
cd /workspaces/ExperimentalDevEnv
make cross                                       # ビルド (aarch64)
make deploy-ec2 EC2=vibecode-graviton            # scp で転送
```
```bash
# EC2 (3 つのターミナル)
~/venv/bin/python3 ~/web-bridge/bridge.py        # Web ブリッジ
sudo ~/cuse_i2c -f --devname=i2c-1               # CUSE I2C スタブ
LD_PRELOAD="~/gpio_shim.so ~/spi_shim.so" ~/sensor_demo
```
Antigravity から EC2 に Remote SSH → PORTS タブで 8080 を Simple Browser で開く。

### RasPi5（実機）

```powershell
# Windows
.\raspi.ps1 deploy                               # Codespaces → Windows → adb push
adb shell
```
```bash
# RasPi5 (adb shell 内)
~/sensor_demo                                    # LD_PRELOAD 不要、実 H/W を直接制御
```

---

## 主要バイナリ

| ファイル | 用途 | EC2 | RasPi5 |
|---|---|---|---|
| `sensor_demo` | 統合デモアプリ（GPIO + I2C OLED + SPI RFID） | ✅（シム経由） | ✅（実機） |
| `gpio_led_button` | GPIO 単機能デモ | ✅ | ✅ |
| `vl53l0x_read` | I2C 距離センサーテスト | ✅（CUSE） | △（実機は要 init 列） |
| `cuse_i2c` | I2C CUSE スタブ（VL53L0X + SSD1306） | EC2 専用 | — |
| `gpio_shim.so` | GPIO LD_PRELOAD シム | EC2 専用 | — |
| `spi_shim.so` | SPI LD_PRELOAD シム（MFRC-522 sim） | EC2 専用 | — |
