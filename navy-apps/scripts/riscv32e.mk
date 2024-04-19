NAVY_HOME = /home/whalefall/Courses/NJU-PA/ics2023/navy-apps
include $(NAVY_HOME)/scripts/riscv/common.mk
CFLAGS  += -march=rv32em -mabi=ilp32e  #overwrite
LDFLAGS += -melf32lriscv
