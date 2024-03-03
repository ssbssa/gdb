
all: libwinipt.a

SRC_DIR = .

CC = gcc
CPPFLAGS = -I$(SRC_DIR)/inc \
	   -DUNICODE -DERROR_IMPLEMENTATION_LIMIT=1292 \
	   -DUFIELD_OFFSET=FIELD_OFFSET
CFLAGS = -O2 -g
AR = ar

SRC = \
      libipt/win32.c \

OBJ = $(SRC:.c=.o)

$(OBJ): %.o: $(SRC_DIR)/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

libwinipt.a: $(OBJ)
	$(AR) rcs $@ $^

clean:
	rm -rf $(OBJ) libwinipt.a
