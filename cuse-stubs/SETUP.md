# CUSEスタブ セットアップ手順

## 全体構成

```
[Codespaces x86_64]  ─クロスコンパイル→  aarch64バイナリ
                                            ↓ scp
[EC2 arm64 (Graviton)]
  bridge.py  ←──Unix socket──  gpio_shim.so (LD_PRELOAD)
  bridge.py  ←──Unix socket──  cuse_i2c     (CUSEデバイス)
  bridge.py  ──WebSocket──→   [Antigravity Simple Browser]
                               http://localhost:8080
```

---

## EC2 の起動 (Windows PowerShell)

```powershell
# c:\VibeCode\ec2.ps1 を使う
.\ec2.ps1 start   # 起動 + SSH config 自動更新
.\ec2.ps1 status  # 状態確認
.\ec2.ps1 stop    # 停止
```

---

## Codespaces でクロスコンパイル

### 1. ツールチェーンのインストール（初回のみ）

```bash
sudo apt install -y gcc-aarch64-linux-gnu libfuse3-dev
```

### 2. gpio_shim.so をビルド

```bash
cd cuse-stubs/gpio-shim
make CC=aarch64-linux-gnu-gcc
file gpio_shim.so   # → ELF 64-bit LSB shared object, ARM aarch64
```

### 3. テストバイナリをビルド

```bash
cd cuse-stubs/test
make CC=aarch64-linux-gnu-gcc   # vl53l0x_read + gpio_led_button の両方
```

### 4. I2C スタブをビルド

```bash
cd cuse-stubs/i2c-stub
make CC=aarch64-linux-gnu-gcc
```

### 5. EC2 に転送

```bash
EC2=vibecode-graviton   # ~/.ssh/config のHost名

scp cuse-stubs/gpio-shim/gpio_shim.so      $EC2:~/
scp cuse-stubs/test/gpio_led_button        $EC2:~/
scp cuse-stubs/test/vl53l0x_read          $EC2:~/
scp cuse-stubs/i2c-stub/cuse_i2c          $EC2:~/
scp -r cuse-stubs/web-bridge              $EC2:~/
```

---

## EC2 で起動する手順

### ターミナル 1: ウェブブリッジ起動

```bash
pip install -r ~/web-bridge/requirements.txt
python3 ~/web-bridge/bridge.py
# → [bridge] Unix socket listening: /tmp/hw_sim.sock
# → [bridge] WebSocket  ws://0.0.0.0:8765
# → [bridge] HTTP panel http://0.0.0.0:8080
```

### ターミナル 2: GPIO デモ実行

```bash
LD_PRELOAD=~/gpio_shim.so ~/gpio_led_button
# → [gpio_shim] loaded, bridge=/tmp/hw_sim.sock
# → GPIO LED+Button demo. Press Ctrl+C to quit.
```

### ターミナル 3 (別に必要な場合): I2C スタブ起動

```bash
sudo ~/cuse_i2c -f --devname=i2c-1
# → /dev/i2c-1 が現れる

# 別シェルで測距テスト
~/vl53l0x_read /dev/i2c-1
```

---

## Antigravity (VSCode) でパネルを開く

Antigravity はポートフォワーディングを自動検出します。

1. **PORTS タブ** で `8080` の行を右クリック → "Open in Simple Browser"
2. HTML パネルが開く（LED・ボタン・距離センサー・RFID・LCD 表示）

> Antigravity から EC2 に SSH 接続している場合、8080/8765 は自動転送されます。  
> `.vscode/settings.json` の `remote.autoForwardPorts: true` が有効になっています。

---

## VSCode タスク

| タスク名 | 動作 |
|---|---|
| `Start Hardware Bridge` | pip install + bridge.py 起動（バックグラウンド） |
| `Run GPIO Demo (EC2)` | LD_PRELOAD=gpio_shim.so で gpio_led_button 実行 |
| `Run VL53L0X Demo (EC2)` | cuse_i2c 起動 + vl53l0x_read 実行 |

---

## カーネル要件の確認 (EC2)

```bash
ls /dev/fuse                             # 存在すればOK
grep -i cuse /boot/config-$(uname -r)   # CONFIG_CUSE=y or m
sudo modprobe cuse                       # m の場合はロード
```

---

## トラブルシューティング

| 症状 | 原因 | 対処 |
|------|------|------|
| `bridge not available` | bridge.py が未起動 | ターミナル1 を確認 |
| `/dev/fuse: Permission denied` | sudo なし | `sudo` で起動、または `usermod -aG fuse $USER` |
| `Model ID: 0x00` | I2C_SLAVE ioctl が届いていない | stderr ログを確認 |
| `double free or corruption` | fuse_opt_parse の aarch64 バグ | `--devname=` 手動パース版を使う (修正済み) |
| GPIO デモが `/dev/gpiochip0: No such file` | shim が未ロード | `LD_PRELOAD=` を確認 |
| SSH config の HostName が古い | IP 変動後の更新漏れ | `.\ec2.ps1 start` が自動更新 |
