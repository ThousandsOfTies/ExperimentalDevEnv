# ExperimentalDevEnv

研究PoC用サンドボックス。CUSEスタブを使ってクラウド上でH/Wをシミュレーションし、
同一バイナリ・同一スクリプトがEC2 arm64と実機(RasPi5)で透過的に動くことを実証する。

## 構成

```
cuse-stubs/
├── i2c-stub/          # CUSE-based /dev/i2c-1 スタブ (VL53L0X ToFセンサーシミュレーション)
│   ├── cuse_i2c.c
│   ├── vl53l0x_sim.c
│   ├── vl53l0x_sim.h
│   └── Makefile
├── test/
│   ├── vl53l0x_read.c # 動確用クライアント（EC2/RasPi5共通バイナリ）
│   └── Makefile
└── SETUP.md           # EC2セットアップ手順
setup_ssh.sh           # CodespacesからEC2へのSSH設定スクリプト
ssh_config.template    # SSH configテンプレート
```

## ターゲット環境

| 環境 | 役割 |
|------|------|
| Codespaces (x86_64) | aarch64クロスコンパイル |
| EC2 t4g.small (arm64) | CUSEスタブで実行・動確 |
| RasPi5 (arm64) | 実機動確 |

## クイックスタート

### 1. Codespacesでクロスコンパイル

```bash
sudo apt install gcc-aarch64-linux-gnu libfuse3-dev

cd cuse-stubs/i2c-stub && make CC=aarch64-linux-gnu-gcc
cd ../test            && make CC=aarch64-linux-gnu-gcc
```

### 2. EC2へ転送・実行

```bash
# SSH設定
bash setup_ssh.sh <EC2_PUBLIC_IP>

# バイナリ転送
scp cuse-stubs/i2c-stub/cuse_i2c  vibecode-graviton:~/
scp cuse-stubs/test/vl53l0x_read  vibecode-graviton:~/

# EC2でスタブ起動 → クライアント実行
ssh vibecode-graviton "sudo modprobe cuse && sudo ./cuse_i2c -f --devname=i2c-1 &"
ssh vibecode-graviton "./vl53l0x_read /dev/i2c-1"
```

詳細は [cuse-stubs/SETUP.md](cuse-stubs/SETUP.md) を参照。
