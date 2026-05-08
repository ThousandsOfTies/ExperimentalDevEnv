# シミュレーション・セットアップ (CUSEスタブ)

このドキュメントでは、クラウド上（AWS EC2 Graviton）で物理ハードウェアをエミュレートする仕組みについて解説します。

## 全体構成

```
[EC2 arm64 (Graviton)]
  bridge.py  ←──Unix socket──  gpio_shim.so (LD_PRELOAD経由のGPIOエミュレート)
  bridge.py  ←──Unix socket──  cuse_i2c     (CUSEによるI2Cデバイススタブ)
  bridge.py  ──WebSocket──→   [Antigravity Simple Browser]
                               http://localhost:8080
```

---

## 各種スタブのビルドと起動手順

Codespaces上で `make cross` を行った後、`make deploy` で EC2 にデプロイしている前提とします。

### ターミナル 1: ウェブブリッジの起動

```bash
# SSH でEC2 に入る
ssh vibecode-graviton

~/venv/bin/python3 ~/web-bridge/bridge.py
# → [bridge] Unix socket listening: /tmp/hw_sim.sock
# → [bridge] WebSocket  ws://0.0.0.0:8765
# → [bridge] HTTP panel http://0.0.0.0:8080
```

### ターミナル 2: GPIO デモの実行 (LD_PRELOAD)

```bash
# SSH でEC2 に入る（別ターミナル）
ssh vibecode-graviton

LD_PRELOAD=~/gpio_shim.so ~/gpio_led_button
# → [gpio_shim] loaded, bridge=/tmp/hw_sim.sock
# → GPIO LED+Button demo. Press Ctrl+C to quit.
```

### ターミナル 3: I2C スタブ起動 (CUSE)

```bash
ssh vibecode-graviton

sudo ~/cuse_i2c -f --devname=i2c-1
# → /dev/i2c-1 が現れる

# 別シェルで測距テスト
~/vl53l0x_read /dev/i2c-1
```

---

## ブラウザパネルへのアクセス

Antigravity から EC2 に Remote SSH 接続している場合、ポートは自動的にフォワードされます。

1. **PORTS タブ** で `8080` の行を右クリック → "Open in Simple Browser" を選択。
2. HTML パネルが開き、LEDの点滅や、ボタンクリックによる状態変化、距離センサーの仮想値の制御が可能になります。

> ポートが自動検出されない場合は手動で `8080` と `8765` を追加してください。

---

## トラブルシューティング

| 症状 | 原因 | 対処 |
|------|------|------|
| `bridge not available` | bridge.py が未起動 | ターミナル1 を確認 |
| `/dev/fuse: Permission denied` | sudo なしでCUSE起動 | `sudo` で起動、または `usermod -aG fuse $USER` |
| GPIO デモが `/dev/gpiochip0: No such file` | shim が未ロード | 実行時に `LD_PRELOAD=~/gpio_shim.so` が付いているか確認 |
| パネルが Disconnected のまま | ポート 8765 未転送 | PORTS タブで 8765 を追加 |
