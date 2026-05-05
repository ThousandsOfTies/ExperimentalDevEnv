# CUSEスタブ セットアップ手順

## 前提

```bash
# EC2 arm64 / RasPi5 で必要なパッケージ
sudo apt install libfuse3-dev fuse3 gcc

# FUSE/CUSEがカーネルに有効か確認
ls /dev/fuse          # 存在すればOK
grep -i cuse /boot/config-$(uname -r)  # CONFIG_CUSE=y or m
```

## Codespaces でクロスコンパイル

```bash
# aarch64ツールチェーンのインストール
sudo apt install gcc-aarch64-linux-gnu libfuse3-dev

# スタブのビルド
cd cuse-stubs/i2c-stub
make CC=aarch64-linux-gnu-gcc

# テストクライアントのビルド
cd ../test
make CC=aarch64-linux-gnu-gcc

# EC2にscp
scp cuse-stubs/i2c-stub/cuse_i2c  ec2-user@<EC2_IP>:~/
scp cuse-stubs/test/vl53l0x_read  ec2-user@<EC2_IP>:~/
```

## EC2 / RasPi5 でスタブを起動

```bash
# ターミナル1: スタブ起動 (フォアグラウンド、ログ表示)
sudo ./cuse_i2c -f --devname=i2c-1

# 確認
ls -la /dev/i2c-1   # 出現すればOK

# ターミナル2: テストクライアント実行
./vl53l0x_read /dev/i2c-1
```

### 期待される出力

```
Model ID: 0xEE  Revision: 0x10
VL53L0X detected. Taking 5 measurements...

  [0] Range:  300 mm  (status=0x01)
  [1] Range:  300 mm  (status=0x01)
  ...
```

## コンテナで使う場合

```bash
docker run --device /dev/fuse --cap-add SYS_ADMIN \
    -v $(pwd):/work ubuntu:22.04 \
    bash -c "cd /work && ./cuse_i2c -f --devname=i2c-1"
```

## 測距値を変える（テスト注入）

`vl53l0x_sim.c` の `simulated_range_mm` を変えるか、
将来的には Unix socket / shared memory で動的注入できるようにする。

## トラブルシューティング

| 症状 | 原因 | 対処 |
|------|------|------|
| `/dev/fuse: Permission denied` | sudoなし | `sudo` で起動、または `usermod -aG fuse $USER` |
| `CUSE` not found in kernel config | CUSEモジュール未ロード | `sudo modprobe cuse` |
| `I2C_RDWR: Argument list too long` | ioctl retry不足 | スタブのiov設計を確認 |
| `Model ID: 0x00` | slave_addrが合っていない | `I2C_SLAVE` ioctlが届いているかstderrで確認 |
