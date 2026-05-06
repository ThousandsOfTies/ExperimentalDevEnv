# cuse-stubs/Makefile — トップレベルビルド
#
# クロスコンパイル (Codespaces x86_64 → EC2 arm64):
#   make cross
#
# ネイティブビルド (EC2 / RasPi5 上で直接):
#   make native
#
# EC2 に転送:
#   make deploy EC2=vibecode-graviton

CC_NATIVE = gcc
CC_CROSS  = aarch64-linux-gnu-gcc

SUBDIRS = gpio-shim i2c-stub test

.PHONY: cross native deploy clean $(SUBDIRS)

cross:
	$(MAKE) -C gpio-shim  CC=$(CC_CROSS)
	$(MAKE) -C i2c-stub   CC=$(CC_CROSS)
	$(MAKE) -C test       CC=$(CC_CROSS)

native:
	$(MAKE) -C gpio-shim  CC=$(CC_NATIVE)
	$(MAKE) -C i2c-stub   CC=$(CC_NATIVE)
	$(MAKE) -C test       CC=$(CC_NATIVE)

clean:
	$(MAKE) -C gpio-shim  clean
	$(MAKE) -C i2c-stub   clean
	$(MAKE) -C test       clean

# EC2=<ssh_host> を指定して実行
deploy:
ifndef EC2
	$(error EC2 変数を指定してください: make deploy EC2=vibecode-graviton)
endif
	scp gpio-shim/gpio_shim.so      $(EC2):~/
	scp test/gpio_led_button        $(EC2):~/
	scp test/vl53l0x_read           $(EC2):~/
	scp i2c-stub/cuse_i2c           $(EC2):~/
	scp -r web-bridge               $(EC2):~/
	@echo "Deploy complete → ssh $(EC2)"
