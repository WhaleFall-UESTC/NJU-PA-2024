CROSS_COMPILE = riscv64-unknown-linux-gnu-
NAVY_HOME = /home/whalefall/Courses/NJU-PA/ics2023/navy-apps
LNK_ADDR = $(if $(VME), 0x40000000, 0x83000000)
CFLAGS  += -fno-pic -march=rv64g -mcmodel=medany
LDFLAGS += --no-relax -Ttext-segment $(LNK_ADDR)
