# Top-level Makefile
#
# クロスコンパイル (Codespaces x86_64 → arm64):
#   make cross
#
# ネイティブビルド:
#   make native
#
# EC2 デプロイ:
#   make deploy-ec2 EC2=vibecode-graviton

CC_CROSS  = aarch64-linux-gnu-gcc
CC_NATIVE = gcc

.PHONY: cross native clean deploy-ec2

cross:
	$(MAKE) -C cuse-stubs cross
	$(MAKE) -C app CC=$(CC_CROSS)

native:
	$(MAKE) -C cuse-stubs native
	$(MAKE) -C app CC=$(CC_NATIVE)

clean:
	$(MAKE) -C cuse-stubs clean
	$(MAKE) -C app clean

deploy-ec2:
ifndef EC2
	$(error EC2 変数を指定してください: make deploy-ec2 EC2=vibecode-graviton)
endif
	$(MAKE) -C cuse-stubs deploy EC2=$(EC2) KEY=$(KEY)
	scp $(if $(KEY),-i $(KEY),) app/sensor_demo $(if $(KEY),ubuntu@$(EC2),$(EC2)):~/
	@echo "App deploy complete"
