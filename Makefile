.PHONY: all clean

CC       = gcc
PREFIX   = /usr/local/bin
TARGET   = smtp-mail
CFLAGS   = -Ideps -Wall
LDFLAGS += `pkg-config --libs libcurl`
SRC      = $(wildcard src/*.c)
SRC     += $(wildcard deps/*/*.c)
OBJS     = $(SRC:.c=.o)

.PHONY:
all: $(TARGET)

.PHONY:
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS)

.PHONY:
%.o: %.c
	$(CC) $(DEP_FLAG) $(CFLAGS) $(LDFLAGS) -o $@ -c $<

.PHONY:
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY:
install: $(TARGET)
	cp -f $(TARGET) $(PREFIX)

.PHONY:
uninstall: $(PREFIX)/$(TARGET)
	rm -f $(PREFIX)/$(TARGET)

src/smtp-mail.o: src/smtp-mail.h src/mdb.h
