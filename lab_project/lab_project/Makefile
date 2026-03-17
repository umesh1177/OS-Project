# ─── Makefile for Lab Management System ───

CC      = gcc
CFLAGS  = -Wall -Wextra -g -pthread
TARGET  = lab_mgmt
SRCS    = main.c login.c computer.c printer.c file_access.c deadlock.c logger.c utils.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean run demo interactive

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Build successful → ./$(TARGET)"

%.o: %.c lab.h
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

demo: $(TARGET)
	./$(TARGET)

interactive: $(TARGET)
	./$(TARGET) -i

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).log lab_activity.log shared_lab_file.txt
	@echo "Cleaned."
