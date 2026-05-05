CC     = gcc
CFLAGS = -Wall -O2
BIN    = vl53l0x_read

$(BIN): vl53l0x_read.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(BIN)

.PHONY: clean
