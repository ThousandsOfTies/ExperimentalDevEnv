/*
 * vl53l0x_read.c — VL53L0X distance readout via /dev/i2c-1
 *
 * Same binary runs on:
 *   - EC2 arm64 (with CUSE stub providing /dev/i2c-1)
 *   - RasPi5    (with physical VL53L0X wired on I2C bus 1)
 *
 * Usage:  ./vl53l0x_read [/dev/i2c-1]
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define VL53L0X_ADDR   0x29

/* Register addresses */
#define REG_MODEL_ID        0xC0
#define REG_REVISION_ID     0xC2
#define REG_SYSRANGE_START  0x00
#define REG_RESULT_INT_STATUS 0x13
#define REG_RESULT_RANGE_STATUS 0x14
#define REG_RANGE_HIGH      0x1E
#define REG_RANGE_LOW       0x1F
#define REG_INT_CLEAR       0x0B

static int fd;

/* Write a single register via I2C_RDWR */
static int write_reg(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = { reg, val };
    struct i2c_msg msg = {
        .addr  = VL53L0X_ADDR,
        .flags = 0,
        .len   = 2,
        .buf   = buf,
    };
    struct i2c_rdwr_ioctl_data data = { .msgs = &msg, .nmsgs = 1 };
    return ioctl(fd, I2C_RDWR, &data);
}

/* Read one or more bytes starting at reg */
static int read_regs(uint8_t reg, uint8_t *out, int len) {
    uint8_t reg_buf = reg;
    struct i2c_msg msgs[2] = {
        { .addr = VL53L0X_ADDR, .flags = 0,        .len = 1,   .buf = &reg_buf },
        { .addr = VL53L0X_ADDR, .flags = I2C_M_RD, .len = len, .buf = out      },
    };
    struct i2c_rdwr_ioctl_data data = { .msgs = msgs, .nmsgs = 2 };
    return ioctl(fd, I2C_RDWR, &data);
}

static uint8_t read_reg(uint8_t reg) {
    uint8_t v = 0;
    read_regs(reg, &v, 1);
    return v;
}

static int poll_interrupt(int max_tries) {
    for (int i = 0; i < max_tries; i++) {
        uint8_t status = read_reg(REG_RESULT_INT_STATUS);
        if (status & 0x07) return 1;
        usleep(10000); /* 10ms */
    }
    return 0;
}

int main(int argc, char *argv[]) {
    const char *devpath = (argc > 1) ? argv[1] : "/dev/i2c-1";

    fd = open(devpath, O_RDWR);
    if (fd < 0) {
        perror(devpath);
        return 1;
    }

    /* Verify device identity */
    uint8_t model_id = read_reg(REG_MODEL_ID);
    uint8_t rev_id   = read_reg(REG_REVISION_ID);
    printf("Model ID: 0x%02X  Revision: 0x%02X\n", model_id, rev_id);
    if (model_id != 0xEE) {
        fprintf(stderr, "Unexpected Model ID (expected 0xEE)\n");
        close(fd);
        return 1;
    }

    printf("VL53L0X detected. Taking 5 measurements...\n\n");

    for (int n = 0; n < 5; n++) {
        /* Trigger single-shot measurement */
        write_reg(REG_SYSRANGE_START, 0x01);

        /* Wait for data-ready interrupt */
        if (!poll_interrupt(50)) {
            fprintf(stderr, "  [%d] Timeout waiting for measurement\n", n);
            continue;
        }

        /* Read range (big-endian 16-bit) */
        uint8_t hi = read_reg(REG_RANGE_HIGH);
        uint8_t lo = read_reg(REG_RANGE_LOW);
        uint16_t range_mm = ((uint16_t)hi << 8) | lo;

        uint8_t status = read_reg(REG_RESULT_RANGE_STATUS);
        printf("  [%d] Range: %4u mm  (status=0x%02X)\n", n, range_mm, status);

        /* Clear interrupt */
        write_reg(REG_INT_CLEAR, 0x01);

        usleep(50000); /* 50ms between shots */
    }

    close(fd);
    return 0;
}
