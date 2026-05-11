# ExperimentalDevEnv — Claude 指示プロンプト

## プロジェクト概要

組み込み H/W シミュレーション開発環境。
Codespaces でクロスコンパイルし、EC2（シミュレーション）または RasPi5（実機）で動かす。
同じバイナリ (`sensor_demo`) が両環境で動作することを実証済み。

---

## ターゲット構成

### EC2（シミュレーション環境）
- インスタンスID: `i-031e0e5f5f1325ddc`、リージョン: `ap-southeast-2`
- SSH Host名: `vibecode-graviton`（`~/.ssh/config` で管理）
- 起動: Windows PowerShell で `.\ec2.ps1 start`（`c:\VibeCode\ec2.ps1`）

### RasPi5（実機）
- IP: `192.168.0.21`（ローカルネットワーク）
- ADB: port `5555`（`adbd` が systemd で自動起動）

### Codespaces（ビルド環境）
- 名前: `glowing-capybara-5j6g4594j75c44j`
- SSH: `gh codespace ssh --codespace glowing-capybara-5j6g4594j75c44j`

---

## デプロイ手順

### 「EC2 にデプロイして」と言われたら

```bash
gh codespace ssh --codespace glowing-capybara-5j6g4594j75c44j -- \
  "cd /workspaces/ExperimentalDevEnv && make cross && make deploy-ec2 EC2=vibecode-graviton"
```

経路: Codespaces → scp → EC2（クラウド同士で直接転送）

### 「実機にデプロイして」と言われたら

1. Codespaces でビルド:
   ```bash
   gh codespace ssh --codespace glowing-capybara-5j6g4594j75c44j -- \
     "cd /workspaces/ExperimentalDevEnv && make cross"
   ```
2. Windows で取得・転送:
   ```powershell
   .\raspi.ps1 deploy
   ```
   経路: Codespaces → gh codespace cp → Windows → adb push → RasPi5

---

## 実行手順

### EC2 でシミュレーション起動（3 プロセス並行）

```bash
ssh vibecode-graviton

# ターミナル①: ブリッジ
~/venv/bin/python3 ~/web-bridge/bridge.py

# ターミナル②: I2C CUSE スタブ
sudo ~/cuse_i2c -f --devname=i2c-1
sudo chmod 666 /dev/i2c-1

# ターミナル③: アプリ本体（シム経由）
LD_PRELOAD="$HOME/gpio_shim.so $HOME/spi_shim.so" ~/sensor_demo
```

Antigravity で Remote SSH → vibecode-graviton → PORTS タブ 8080 を Simple Browser で開く。

### RasPi5 で実機実行

```powershell
adb shell
```
```bash
# adb shell 内
~/sensor_demo
```

---

## EC2 の起動・停止

```powershell
.\ec2.ps1 start   # 起動 + SSH config 自動更新 + リポジトリ自動 pull
.\ec2.ps1 stop    # 停止
.\ec2.ps1 status  # 状態確認
```

---

## ビルド成果物

| ファイル | 用途 |
|---|---|
| `app/sensor_demo` | 統合デモアプリ（GPIO + I2C OLED + SPI RFID） |
| `cuse-stubs/gpio-shim/gpio_shim.so` | GPIO LD_PRELOAD シム（EC2 用） |
| `cuse-stubs/spi-shim/spi_shim.so` | SPI LD_PRELOAD シム（MFRC-522 sim、EC2 用） |
| `cuse-stubs/i2c-stub/cuse_i2c` | I2C CUSE スタブ（VL53L0X + SSD1306、EC2 用） |
| `cuse-stubs/test/gpio_led_button` | GPIO 単機能デモ |
| `cuse-stubs/test/vl53l0x_read` | VL53L0X 距離センサーテスト |
| `cuse-stubs/web-bridge/` | Web ブリッジ + HTML パネル |
