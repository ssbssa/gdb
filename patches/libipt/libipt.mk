
all: libipt.a

SRC_DIR = .
PT_VERSION_MAJOR = 0
PT_VERSION_MINOR = 0
PT_VERSION_PATCH = 0

CC = gcc
CPPFLAGS = -I$(SRC_DIR)/include -I$(SRC_DIR)/internal/include \
	   -I$(SRC_DIR)/../include -I$(SRC_DIR)/internal/include/windows \
	   -DPT_VERSION_MAJOR=$(PT_VERSION_MAJOR) \
	   -DPT_VERSION_MINOR=$(PT_VERSION_MINOR) \
	   -DPT_VERSION_PATCH=$(PT_VERSION_PATCH) \
	   -DPT_VERSION_BUILD=0 -DPT_VERSION_EXT=\"\"
CFLAGS = -O2 -g
AR = ar

SRC = \
      src/pt_asid.c \
      src/pt_block_cache.c \
      src/pt_block_decoder.c \
      src/pt_config.c \
      src/pt_cpu.c \
      src/pt_encoder.c \
      src/pt_error.c \
      src/pt_event_decoder.c \
      src/pt_event_queue.c \
      src/pt_ild.c \
      src/pt_image.c \
      src/pt_image_section_cache.c \
      src/pt_insn.c \
      src/pt_insn_decoder.c \
      src/pt_last_ip.c \
      src/pt_msec_cache.c \
      src/pt_packet.c \
      src/pt_packet_decoder.c \
      src/pt_query_decoder.c \
      src/pt_retstack.c \
      src/pt_section.c \
      src/pt_section_file.c \
      src/pt_sync.c \
      src/pt_time.c \
      src/pt_tnt_cache.c \
      src/pt_version.c \
      src/windows/pt_section_windows.c \

OBJ = $(SRC:.c=.o)

$(OBJ): %.o: $(SRC_DIR)/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

libipt.a: $(OBJ)
	$(AR) rcs $@ $^

clean:
	rm -rf $(OBJ) libipt.a
