
MYPKG=ssbssa-1
BUILD_BITS=32

SOURCE_DIR=src
SOURCE_DIR_ABS=$(abspath $(SOURCE_DIR))
BUILD_DIR=build$(BUILD_BITS)
BUILD_DIR_ABS=$(abspath $(BUILD_DIR))

GDB_LIBS=$(abspath gdb-libs$(BUILD_BITS))
GDB_DIR=$(abspath gdb$(BUILD_BITS))
BINUTILS_DIR=$(abspath binutils$(BUILD_BITS))

ifeq ($(BUILD_BITS),32)
  MYBUILD=i686-w64-mingw32
  MYTARGET=i686-w64-mingw32
  CROSS_CONF=
else ifeq ($(BUILD_BITS),64)
  MYBUILD=i686-w64-mingw32
  MYTARGET=x86_64-w64-mingw32
  CROSS_CONF=CC=$(MYTARGET)-gcc CXX=$(MYTARGET)-g++ LIBEXE=$(MYTARGET)-ar
else
  $(error BUILD_BITS is $(BUILD_BITS))
endif


EXPAT_VER=2.3.0
EXPAT_SRC_DIR=expat-$(EXPAT_VER)
EXPAT_FILE=$(EXPAT_SRC_DIR).tar.xz
EXPAT_CONF=$(SOURCE_DIR_ABS)/$(EXPAT_SRC_DIR)/configure \
	   --build=$(MYBUILD) --host=$(MYTARGET) \
	   --enable-static --disable-shared --prefix=$(GDB_LIBS)

PDCURSES_VER=3.4
PDCURSES_SRC_DIR=PDCurses-$(PDCURSES_VER)
PDCURSES_FILE=$(PDCURSES_SRC_DIR).tar.gz
PDCURSES_CONF=$(SOURCE_DIR_ABS)/$(PDCURSES_SRC_DIR)/configure \
	      --build=$(MYBUILD) --host=$(MYTARGET) \
	      --enable-static --disable-shared --prefix=$(GDB_LIBS)

ICONV_VER=1.16
ICONV_SRC_DIR=libiconv-$(ICONV_VER)
ICONV_FILE=$(ICONV_SRC_DIR).tar.gz
ICONV_CONF=$(SOURCE_DIR_ABS)/$(ICONV_SRC_DIR)/configure \
	   --build=$(MYBUILD) --host=$(MYTARGET) \
	   --enable-static --disable-shared --prefix=$(GDB_LIBS)

PYTHON_VER=2.7.13
PYTHON_FILE=python-$(PYTHON_VER)-w$(BUILD_BITS).tar.xz
PYTHON_DIR=Python27

BOOST_VER=1_69_0
BOOST_SRC_DIR=boost_$(BOOST_VER)
BOOST_FILE=$(BOOST_SRC_DIR).tar.bz2

SOURCE_HIGHLIGHT_VER=3.1.9
SOURCE_HIGHLIGHT_SRC_DIR=source-highlight-$(SOURCE_HIGHLIGHT_VER)
SOURCE_HIGHLIGHT_FILE=$(SOURCE_HIGHLIGHT_SRC_DIR).tar.gz
SOURCE_HIGHLIGHT_CONF=$(SOURCE_DIR_ABS)/$(SOURCE_HIGHLIGHT_SRC_DIR)/configure \
		      --build=$(MYBUILD) --host=$(MYTARGET) \
		      --with-boost=$(GDB_LIBS) \
		      --enable-static --disable-shared --prefix=$(GDB_LIBS)

LZMA_VER=5.2.5
LZMA_SRC_DIR=xz-$(LZMA_VER)
LZMA_FILE=$(LZMA_SRC_DIR).tar.xz
LZMA_CONF=$(SOURCE_DIR_ABS)/$(LZMA_SRC_DIR)/configure \
	  --build=$(MYBUILD) --host=$(MYTARGET) \
	  --enable-static --disable-shared --prefix=$(GDB_LIBS)

GMP_VER=6.1.2
GMP_SRC_DIR=gmp-$(GMP_VER)
GMP_FILE=$(GMP_SRC_DIR).tar.xz
GMP_CONF=$(SOURCE_DIR_ABS)/$(GMP_SRC_DIR)/configure \
	 --build=$(MYBUILD) --host=$(MYTARGET) \
	 --enable-static --disable-shared --prefix=$(GDB_LIBS)

MPFR_VER=3.1.6
MPFR_SRC_DIR=mpfr-$(MPFR_VER)
MPFR_FILE=$(MPFR_SRC_DIR).tar.xz
MPFR_CONF=$(SOURCE_DIR_ABS)/$(MPFR_SRC_DIR)/configure \
	 --build=$(MYBUILD) --host=$(MYTARGET) \
	 --enable-static --disable-shared --prefix=$(GDB_LIBS) \
	 --with-gmp-build=$(BUILD_DIR_ABS)/gmp

GDB_VER=8.1.1
GDB_SRC_DIR=gdb-$(GDB_VER)
GDB_FILE=$(GDB_SRC_DIR).tar.xz
GDB_CONF=$(SOURCE_DIR_ABS)/$(GDB_SRC_DIR)/configure \
	 --build=$(MYBUILD) --host=$(MYTARGET) --target=$(MYTARGET) \
	 --disable-nls \
	 CPPFLAGS="-I$(GDB_LIBS)/include" LDFLAGS="-L$(GDB_LIBS)/lib" \
	 --enable-curses --enable-tui \
	 --with-libiconv-prefix=$(GDB_LIBS) \
	 --disable-install-libbfd --disable-install-libiberty \
	 --with-pkgversion=$(MYPKG)
SET_PKG_PATH=export PKG_CONFIG_PATH="$(GDB_LIBS)/lib/pkgconfig";
GDB_GIT_DIR=/c/src/repos/binutils-gdb.git
GDB_GIT_CONF=$(GDB_GIT_DIR)/configure \
	 --build=$(MYBUILD) --host=$(MYTARGET) --target=$(MYTARGET) \
	 --disable-nls \
	 CPPFLAGS="-I$(GDB_LIBS)/include" LDFLAGS="-L$(GDB_LIBS)/lib" \
	 --enable-curses --enable-tui \
	 --with-libiconv-prefix=$(GDB_LIBS) \
	 --with-liblzma-prefix=$(GDB_LIBS) \
	 --disable-install-libbfd --disable-install-libiberty \
	 --disable-binutils --disable-gas --disable-gprof --disable-ld \
	 --with-pkgversion=$(MYPKG)
GDB_TEST_CONF=/c/src/repos/gdb-testsuite/configure \
	 --build=$(MYBUILD) --host=$(MYTARGET) --target=$(MYTARGET) \
	 --disable-nls \
	 CPPFLAGS="-I$(GDB_LIBS)/include" LDFLAGS="-L$(GDB_LIBS)/lib" \
	 --enable-curses --enable-tui \
	 --with-libiconv-prefix=$(GDB_LIBS) \
	 --with-liblzma-prefix=$(GDB_LIBS) \
	 --disable-install-libbfd --disable-install-libiberty \
	 --disable-binutils --disable-gas --disable-gprof --disable-ld \
	 --with-pkgversion=$(MYPKG)
GDB_REDHAT64_CONF=$(GDB_GIT_DIR)/configure \
	 --build=$(MYBUILD) --host=$(MYBUILD) --target=x86_64-redhat-linux \
	 --disable-nls \
	 CPPFLAGS="-I/gdb/gdb-libs32/include" LDFLAGS="-L/gdb/gdb-libs32/lib" \
	 --disable-curses --disable-tui \
	 --with-libiconv-prefix=$(GDB_LIBS) \
	 --disable-install-libbfd --disable-install-libiberty \
	 --disable-binutils --disable-gas --disable-gprof --disable-ld \
	 --with-sysroot=$(GDB_DIR)-redhat64/$(MYTARGET)/sys-root
GDB_REDHAT32_HOST_CONF=$(GDB_GIT_DIR)/configure \
	 --build=$(MYBUILD) --host=i686-redhat-linux \
	 --disable-nls \
	 --disable-curses --disable-tui \
	 --disable-install-libbfd --disable-install-libiberty \
	 --disable-binutils --disable-gas --disable-gprof --disable-ld

BINUTILS_GIT_CONF=$(GDB_GIT_DIR)/configure \
	 --build=$(MYBUILD) --host=$(MYBUILD) --target=$(MYTARGET) \
	 --disable-multilib --disable-nls --with-sysroot=$(BINUTILS_DIR) \
	 --prefix=$(BINUTILS_DIR) --enable-targets=$(MYTARGET) \
	 --disable-install-libbfd --disable-install-libiberty \
	 --disable-gdb --disable-libdecnumber --disable-readline --disable-sim \
	 --with-pkgversion=$(MYPKG)


all:
all: $(BUILD_DIR)/expat-05-make-install.done
all: $(BUILD_DIR)/pdcurses-04-make-install.done
all: $(BUILD_DIR)/iconv-05-make-install.done
all: $(BUILD_DIR)/gdb-05-make-install.done


$(SOURCE_DIR):
	@mkdir $@

$(BUILD_DIR):
	@mkdir $@


# expat

$(SOURCE_DIR)/expat-01-extract.done: | $(SOURCE_DIR) pkg/$(EXPAT_FILE)
	tar -C $(SOURCE_DIR) -xJf pkg/$(EXPAT_FILE)
	@touch $@

$(BUILD_DIR)/expat-03-configure.done: | $(SOURCE_DIR)/expat-01-extract.done
	@mkdir -p $(BUILD_DIR)/expat
	cd $(BUILD_DIR)/expat && $(EXPAT_CONF)
	@touch $@

$(BUILD_DIR)/expat-04-make.done: | $(BUILD_DIR)/expat-03-configure.done
	$(MAKE) -C $(BUILD_DIR)/expat
	@touch $@

$(BUILD_DIR)/expat-05-make-install.done: | $(BUILD_DIR)/expat-04-make.done
	$(MAKE) -C $(BUILD_DIR)/expat install
	@touch $@


# pdcurses

$(SOURCE_DIR)/pdcurses-01-extract.done: | pkg/$(PDCURSES_FILE) $(SOURCE_DIR)/expat-01-extract.done
	tar -C $(SOURCE_DIR) -xzf pkg/$(PDCURSES_FILE)
	@touch $@

$(SOURCE_DIR)/pdcurses-02-patch-01-tputs.done: | $(SOURCE_DIR)/pdcurses-01-extract.done
	patch -d $(SOURCE_DIR)/$(PDCURSES_SRC_DIR) -p1 <patches/pdcurses/0001-fix-tputs.patch
	@touch $@

$(SOURCE_DIR)/pdcurses-02-patch-02-save-screen.done: | $(SOURCE_DIR)/pdcurses-02-patch-01-tputs.done
	patch -d $(SOURCE_DIR)/$(PDCURSES_SRC_DIR) -p1 <patches/pdcurses/0002-always-save-screen-when-entering-curses-mode.patch
	@touch $@

$(SOURCE_DIR)/pdcurses-02-patch-03-save-full-screen.done: | $(SOURCE_DIR)/pdcurses-02-patch-02-save-screen.done
	patch -d $(SOURCE_DIR)/$(PDCURSES_SRC_DIR) -p1 <patches/pdcurses/0003-save-full-screen-buffer.patch
	@touch $@

$(SOURCE_DIR)/pdcurses-02-patch-04-debug.done: | $(SOURCE_DIR)/pdcurses-02-patch-03-save-full-screen.done
	patch -d $(SOURCE_DIR)/$(PDCURSES_SRC_DIR) -p1 <patches/pdcurses/0004-add-debug-information-in-release-build.patch
	@touch $@

$(SOURCE_DIR)/pdcurses-02-patch-05-no-keypad.done: | $(SOURCE_DIR)/pdcurses-02-patch-04-debug.done
	patch -d $(SOURCE_DIR)/$(PDCURSES_SRC_DIR) -p1 <patches/pdcurses/0005-no-keypad.patch
	@touch $@

$(SOURCE_DIR)/pdcurses-02-patch-06-ctrl-left-right.done: | $(SOURCE_DIR)/pdcurses-02-patch-05-no-keypad.done
	patch -d $(SOURCE_DIR)/$(PDCURSES_SRC_DIR) -p1 <patches/pdcurses/0006-ctrl-left-right.patch
	@touch $@

$(SOURCE_DIR)/pdcurses-02-patch-07-clear-page.done: | $(SOURCE_DIR)/pdcurses-02-patch-06-ctrl-left-right.done
	patch -d $(SOURCE_DIR)/$(PDCURSES_SRC_DIR) -p1 <patches/pdcurses/0007-clear-page.patch
	@touch $@

$(SOURCE_DIR)/pdcurses-02-patch-08-line-up.done: | $(SOURCE_DIR)/pdcurses-02-patch-07-clear-page.done
	patch -d $(SOURCE_DIR)/$(PDCURSES_SRC_DIR) -p1 <patches/pdcurses/0008-line-up.patch
	@touch $@

$(SOURCE_DIR)/pdcurses-02-patch-09-doupdate.done: | $(SOURCE_DIR)/pdcurses-02-patch-08-line-up.done
	patch -d $(SOURCE_DIR)/$(PDCURSES_SRC_DIR) -p1 <patches/pdcurses/0009-fix-doupdate.patch
	@touch $@

$(SOURCE_DIR)/pdcurses-02-patch-10-fix-wheel.done: | $(SOURCE_DIR)/pdcurses-02-patch-09-doupdate.done
	patch -d $(SOURCE_DIR)/$(PDCURSES_SRC_DIR) -p1 <patches/pdcurses/0010-fix-wheel.patch
	@touch $@

$(SOURCE_DIR)/pdcurses-02-patch-11-processed-input.done: | $(SOURCE_DIR)/pdcurses-02-patch-10-fix-wheel.done
	patch -d $(SOURCE_DIR)/$(PDCURSES_SRC_DIR) -p1 <patches/pdcurses/0011-processed-input.patch
	@touch $@

$(SOURCE_DIR)/pdcurses-02-patch-12-wheel-coordinates.done: | $(SOURCE_DIR)/pdcurses-02-patch-11-processed-input.done
	patch -d $(SOURCE_DIR)/$(PDCURSES_SRC_DIR) -p1 <patches/pdcurses/0012-wheel-coordinates.patch
	@touch $@

$(SOURCE_DIR)/pdcurses-02-patch-13-ncurses-mouse-api.done: | $(SOURCE_DIR)/pdcurses-02-patch-12-wheel-coordinates.done
	patch -d $(SOURCE_DIR)/$(PDCURSES_SRC_DIR) -p1 <patches/pdcurses/0013-ncurses-mouse-api.patch
	@touch $@

$(BUILD_DIR)/pdcurses-03-make.done: | $(BUILD_DIR)/expat-05-make-install.done $(SOURCE_DIR)/pdcurses-02-patch-13-ncurses-mouse-api.done
	@mkdir -p $(BUILD_DIR)/pdcurses
	$(MAKE) -C $(BUILD_DIR)/pdcurses -f $(SOURCE_DIR_ABS)/$(PDCURSES_SRC_DIR)/win32/gccwin32.mak $(CROSS_CONF) PDCURSES_SRCDIR=$(SOURCE_DIR_ABS)/$(PDCURSES_SRC_DIR) pdcurses.a
	@touch $@

$(BUILD_DIR)/pdcurses-04-make-install.done: | $(BUILD_DIR)/pdcurses-03-make.done
	$(SOURCE_DIR_ABS)/$(PDCURSES_SRC_DIR)/install-sh -d -m 755 $(GDB_LIBS)/include $(GDB_LIBS)/lib
	cd $(SOURCE_DIR_ABS)/$(PDCURSES_SRC_DIR) && ./install-sh -c -m 644 curses.h $(GDB_LIBS)/include/curses.h
	cd $(SOURCE_DIR_ABS)/$(PDCURSES_SRC_DIR) && ./install-sh -c -m 644 term.h $(GDB_LIBS)/include/term.h
	cd $(BUILD_DIR)/pdcurses && $(SOURCE_DIR_ABS)/$(PDCURSES_SRC_DIR)/install-sh -c -m 644 pdcurses.a $(GDB_LIBS)/lib/libcurses.a
	@touch $@

# iconv

$(SOURCE_DIR)/iconv-01-extract.done: | pkg/$(ICONV_FILE) $(SOURCE_DIR)/pdcurses-01-extract.done
	tar -C $(SOURCE_DIR) -xzf pkg/$(ICONV_FILE)
	@touch $@

$(BUILD_DIR)/iconv-03-configure.done: | $(BUILD_DIR)/expat-03-configure.done $(BUILD_DIR)/pdcurses-04-make-install.done $(SOURCE_DIR)/iconv-01-extract.done
	@mkdir -p $(BUILD_DIR)/iconv
	cd $(BUILD_DIR)/iconv && $(ICONV_CONF)
	@touch $@

$(BUILD_DIR)/iconv-04-make.done: | $(BUILD_DIR)/iconv-03-configure.done
	$(MAKE) -C $(BUILD_DIR)/iconv
	@touch $@

$(BUILD_DIR)/iconv-05-make-install.done: | $(BUILD_DIR)/iconv-04-make.done
	$(MAKE) -C $(BUILD_DIR)/iconv install
	@touch $@


# python

$(GDB_LIBS)/$(PYTHON_DIR): | pkg/$(PYTHON_FILE) $(BUILD_DIR)/iconv-05-make-install.done
	tar -C $(GDB_LIBS) -xJf pkg/$(PYTHON_FILE)


# boost

$(SOURCE_DIR)/boost-01-extract.done: | pkg/$(BOOST_FILE) $(SOURCE_DIR)/iconv-01-extract.done
	tar -C $(SOURCE_DIR) -xjf pkg/$(BOOST_FILE)
	@touch $@

$(BUILD_DIR)/boost-02-headers.done: | $(SOURCE_DIR)/boost-01-extract.done
	@mkdir -p $(GDB_LIBS)/include
	cp -R $(SOURCE_DIR)/$(BOOST_SRC_DIR)/boost $(GDB_LIBS)/include/boost
	@touch $@

$(BUILD_DIR)/boost-03-regex.done: | $(BUILD_DIR)/boost-02-headers.done
	@mkdir -p $(BUILD_DIR)/boost-regex $(GDB_LIBS)/lib
	cp patches/boost/regex.mk $(BUILD_DIR)/boost-regex/Makefile
	$(MAKE) -C $(BUILD_DIR)/boost-regex $(CROSS_CONF) SRC_DIR=$(SOURCE_DIR_ABS)/$(BOOST_SRC_DIR)/libs/regex/src INC_DIR=$(GDB_LIBS)/include
	cp $(BUILD_DIR)/boost-regex/libboost_regex.a $(GDB_LIBS)/lib/
	@touch $@


# source-highlight

$(SOURCE_DIR)/source-highlight-01-extract.done: | pkg/$(SOURCE_HIGHLIGHT_FILE) $(SOURCE_DIR)/boost-01-extract.done $(BUILD_DIR)/boost-03-regex.done
	tar -C $(SOURCE_DIR) -xzf pkg/$(SOURCE_HIGHLIGHT_FILE)
	@touch $@

$(SOURCE_DIR)/source-highlight-02-patch-01-colors.done: | $(SOURCE_DIR)/source-highlight-01-extract.done
	patch -d $(SOURCE_DIR)/$(SOURCE_HIGHLIGHT_SRC_DIR) -p0 <patches/source-hightlight/console-colors.patch
	@touch $@

$(SOURCE_DIR)/source-highlight-02-patch-02-remove-throw.done: | $(SOURCE_DIR)/source-highlight-02-patch-01-colors.done
	patch -d $(SOURCE_DIR)/$(SOURCE_HIGHLIGHT_SRC_DIR) -p1 <patches/source-hightlight/Remove-throw-specifications.patch
	@touch $@

$(BUILD_DIR)/source-highlight-03-configure.done: | $(SOURCE_DIR)/source-highlight-02-patch-02-remove-throw.done $(BUILD_DIR)/boost-03-regex.done
	@mkdir -p $(BUILD_DIR)/source-highlight
	cd $(BUILD_DIR)/source-highlight && $(SOURCE_HIGHLIGHT_CONF)
	@touch $@

$(BUILD_DIR)/source-highlight-04-make.done: | $(BUILD_DIR)/source-highlight-03-configure.done
	$(MAKE) -C $(BUILD_DIR)/source-highlight
	@touch $@

$(BUILD_DIR)/source-highlight-05-make-install.done: | $(BUILD_DIR)/source-highlight-04-make.done
	$(MAKE) -C $(BUILD_DIR)/source-highlight install
	@touch $@


# lzma

$(SOURCE_DIR)/lzma-01-extract.done: | pkg/$(LZMA_FILE) $(SOURCE_DIR)/source-highlight-01-extract.done
	tar -C $(SOURCE_DIR) -xJf pkg/$(LZMA_FILE)
	@touch $@

$(BUILD_DIR)/lzma-03-configure.done: | $(SOURCE_DIR)/lzma-01-extract.done
	@mkdir -p $(BUILD_DIR)/lzma
	cd $(BUILD_DIR)/lzma && $(LZMA_CONF)
	@touch $@

$(BUILD_DIR)/lzma-04-make.done: | $(BUILD_DIR)/lzma-03-configure.done
	$(MAKE) -C $(BUILD_DIR)/lzma/src/liblzma
	@touch $@

$(BUILD_DIR)/lzma-05-make-install.done: | $(BUILD_DIR)/lzma-04-make.done
	$(MAKE) -C $(BUILD_DIR)/lzma/src/liblzma install
	@touch $@


# gmp

$(SOURCE_DIR)/gmp-01-extract.done: | pkg/$(GMP_FILE) $(SOURCE_DIR)/lzma-01-extract.done
	tar -C $(SOURCE_DIR) -xJf pkg/$(GMP_FILE)
	@touch $@

$(BUILD_DIR)/gmp-03-configure.done: | $(SOURCE_DIR)/gmp-01-extract.done
	@mkdir -p $(BUILD_DIR)/gmp
	cd $(BUILD_DIR)/gmp && $(GMP_CONF)
	@touch $@

$(BUILD_DIR)/gmp-04-make.done: | $(BUILD_DIR)/gmp-03-configure.done
	$(MAKE) -C $(BUILD_DIR)/gmp
	@touch $@

$(BUILD_DIR)/gmp-05-make-install.done: | $(BUILD_DIR)/gmp-04-make.done
	$(MAKE) -C $(BUILD_DIR)/gmp install
	@touch $@


# mpfr

$(SOURCE_DIR)/mpfr-01-extract.done: | pkg/$(MPFR_FILE) $(SOURCE_DIR)/gmp-01-extract.done
	tar -C $(SOURCE_DIR) -xJf pkg/$(MPFR_FILE)
	@touch $@

$(BUILD_DIR)/mpfr-03-configure.done: | $(SOURCE_DIR)/mpfr-01-extract.done $(BUILD_DIR)/gmp-04-make.done
	@mkdir -p $(BUILD_DIR)/mpfr
	cd $(BUILD_DIR)/mpfr && $(MPFR_CONF)
	@touch $@

$(BUILD_DIR)/mpfr-04-make.done: | $(BUILD_DIR)/mpfr-03-configure.done $(BUILD_DIR)/gmp-05-make-install.done
	$(MAKE) -C $(BUILD_DIR)/mpfr
	@touch $@

$(BUILD_DIR)/mpfr-05-make-install.done: | $(BUILD_DIR)/mpfr-04-make.done
	$(MAKE) -C $(BUILD_DIR)/mpfr install
	@touch $@


# gdb

$(SOURCE_DIR)/gdb-01-extract.done: | pkg/$(GDB_FILE) $(SOURCE_DIR)/mpfr-01-extract.done
	tar -C $(SOURCE_DIR) -xJf pkg/$(GDB_FILE)
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-01-jit-installer.done: | $(SOURCE_DIR)/gdb-01-extract.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0001-Add-install-uninstall-commands-for-JIT-debugger.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-02-doc.done: | $(SOURCE_DIR)/gdb-02-patch-01-jit-installer.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0002-Only-build-missing-texi-files.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-03-tui-syntax-highlight.done: | $(SOURCE_DIR)/gdb-02-patch-02-doc.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0003-Add-syntax-highlighting-for-TUI.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-04-clear-symbols.done: | $(SOURCE_DIR)/gdb-02-patch-03-tui-syntax-highlight.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0004-Clear-symbols-if-executable-can-t-be-attached.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-05-thiscall.done: | $(SOURCE_DIR)/gdb-02-patch-04-clear-symbols.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0005-Use-thiscall-calling-convention-for-class-members.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-06-userprofile-home.done: | $(SOURCE_DIR)/gdb-02-patch-05-thiscall.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0006-Use-USERPROFILE-as-alternative-to-HOME.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-07-access-violation.done: | $(SOURCE_DIR)/gdb-02-patch-06-userprofile-home.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0007-Show-details-for-access-violation.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-08-tui-multi-line-syntax.done: | $(SOURCE_DIR)/gdb-02-patch-07-access-violation.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0008-Add-multi-line-syntax-highlighting-for-TUI.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-09-console-scroll.done: | $(SOURCE_DIR)/gdb-02-patch-08-tui-multi-line-syntax.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0009-Use-page-up-down-to-scroll-in-console-buffer.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-10-resize-crashes.done: | $(SOURCE_DIR)/gdb-02-patch-09-console-scroll.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0010-Fix-resize-crashes.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-11-tui-search.done: | $(SOURCE_DIR)/gdb-02-patch-10-resize-crashes.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0011-Fix-search-for-TUI.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-12-ctrl-left-right.done: | $(SOURCE_DIR)/gdb-02-patch-11-tui-search.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0012-Use-ctrl-left-right-to-move-to-previous-next-word.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-13-moving-cursor.done: | $(SOURCE_DIR)/gdb-02-patch-12-ctrl-left-right.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0013-Display-cursor-when-moving.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-14-exec-point-highlight.done: | $(SOURCE_DIR)/gdb-02-patch-13-moving-cursor.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0014-Don-t-highlight-wrong-execution-point.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-15-no-warn-debuglink.done: | $(SOURCE_DIR)/gdb-02-patch-14-exec-point-highlight.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0015-Don-t-warn-for-debuglink-section.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-16-no-source-color.done: | $(SOURCE_DIR)/gdb-02-patch-15-no-warn-debuglink.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0016-Fix-highlight-colors-for-empty-source-window.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-17-console-resize.done: | $(SOURCE_DIR)/gdb-02-patch-16-no-source-color.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0017-Add-console-command-to-resize-console-window.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-18-tui-list-frame.done: | $(SOURCE_DIR)/gdb-02-patch-17-console-resize.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0018-Restore-TUI-behavior-of-list-and-frame.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-19-tui-wheel.done: | $(SOURCE_DIR)/gdb-02-patch-18-tui-list-frame.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0019-Use-mouse-wheel-in-TUI.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-20-thread-name.done: | $(SOURCE_DIR)/gdb-02-patch-19-tui-wheel.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0020-Support-thread-names-in-gdbserver.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-21-readline-assert.done: | $(SOURCE_DIR)/gdb-02-patch-20-thread-name.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0021-Fix-readline-assert.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-22-memory-leaks.done: | $(SOURCE_DIR)/gdb-02-patch-21-readline-assert.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0022-Fix-memory-leaks.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-23-scrolling-tui.done: | $(SOURCE_DIR)/gdb-02-patch-22-memory-leaks.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0023-Fix-scrolling-in-TUI.patch
	@touch $@

$(SOURCE_DIR)/gdb-02-patch-24-no-segment-registers-win64.done: | $(SOURCE_DIR)/gdb-02-patch-23-scrolling-tui.done
	patch -d $(SOURCE_DIR)/$(GDB_SRC_DIR) -p1 <patches/gdb/0024-No-segment-registers-for-win64.patch
	@touch $@

$(BUILD_DIR)/gdb-03-configure.done: | $(BUILD_DIR)/expat-05-make-install.done $(BUILD_DIR)/pdcurses-04-make-install.done $(BUILD_DIR)/iconv-05-make-install.done $(SOURCE_DIR)/gdb-02-patch-24-no-segment-registers-win64.done
	@mkdir -p $(BUILD_DIR)/gdb
	cd $(BUILD_DIR)/gdb && $(GDB_CONF) --prefix=$(GDB_DIR)
	@touch $@

$(BUILD_DIR)/gdb-04-make.done: | $(BUILD_DIR)/gdb-03-configure.done
	$(MAKE) CC_FOR_BUILD=$(MYBUILD)-gcc -C $(BUILD_DIR)/gdb
	@touch $@

$(BUILD_DIR)/gdb-05-make-install.done: | $(BUILD_DIR)/gdb-04-make.done
	$(MAKE) -C $(BUILD_DIR)/gdb/gdb install-strip
	@touch $@


# gdb-python

$(BUILD_DIR)/gdb-python-01-configure.done: | $(BUILD_DIR)/expat-05-make-install.done $(BUILD_DIR)/pdcurses-04-make-install.done $(BUILD_DIR)/iconv-05-make-install.done $(SOURCE_DIR)/gdb-02-patch-24-no-segment-registers-win64.done $(GDB_LIBS)/$(PYTHON_DIR)
	@mkdir -p $(BUILD_DIR)/gdb-python
	cd $(BUILD_DIR)/gdb-python && $(GDB_CONF) --prefix=$(GDB_DIR)-python --with-python=$(GDB_LIBS)/$(PYTHON_DIR)/python
	@touch $@

$(BUILD_DIR)/gdb-python-02-make.done: | $(BUILD_DIR)/gdb-python-01-configure.done
	$(MAKE) CC_FOR_BUILD=$(MYBUILD)-gcc -C $(BUILD_DIR)/gdb-python
	@touch $@

$(BUILD_DIR)/gdb-python-03-make-install.done: | $(BUILD_DIR)/gdb-python-02-make.done
	$(MAKE) -C $(BUILD_DIR)/gdb-python/gdb install-strip
	@touch $@

$(BUILD_DIR)/gdb-python-04-python.done: | $(BUILD_DIR)/gdb-python-03-make-install.done
	cp -af $(GDB_LIBS)/$(PYTHON_DIR)/python27.dll $(GDB_DIR)-python/bin/
	cp -arf $(GDB_LIBS)/$(PYTHON_DIR)/Lib $(GDB_DIR)-python/lib
	rm -rf $(GDB_DIR)-python/lib/test
	@touch $@


# gdb-git

$(BUILD_DIR)/gdb-git-01-configure.done: | $(BUILD_DIR)/expat-05-make-install.done $(BUILD_DIR)/pdcurses-04-make-install.done $(BUILD_DIR)/iconv-05-make-install.done $(BUILD_DIR)/boost-03-regex.done $(BUILD_DIR)/source-highlight-05-make-install.done $(BUILD_DIR)/lzma-05-make-install.done $(BUILD_DIR)/gmp-05-make-install.done $(BUILD_DIR)/mpfr-05-make-install.done
	@mkdir -p $(BUILD_DIR)/gdb-git
	$(SET_PKG_PATH) cd $(BUILD_DIR)/gdb-git && $(GDB_GIT_CONF) --prefix=$(GDB_DIR)-git
	@touch $@

$(BUILD_DIR)/gdb-git-02-make.done: | $(BUILD_DIR)/gdb-git-01-configure.done
	$(SET_PKG_PATH) $(MAKE) CC_FOR_BUILD=$(MYBUILD)-gcc -C $(BUILD_DIR)/gdb-git
	@touch $@

$(BUILD_DIR)/gdb-git-03-make-install.done: | $(BUILD_DIR)/gdb-git-02-make.done
	$(SET_PKG_PATH) $(MAKE) -C $(BUILD_DIR)/gdb-git/gdb install-strip
	$(SET_PKG_PATH) $(MAKE) -C $(BUILD_DIR)/gdb-git/gdbserver install-strip
	@touch $@

$(BUILD_DIR)/gdb-git-04-source-highlight.done: | $(BUILD_DIR)/gdb-git-03-make-install.done
	cp -R $(GDB_LIBS)/share/source-highlight $(GDB_DIR)-git/share/source-highlight
	@touch $@

$(BUILD_DIR)/gdb-git-05-licenses.done: | $(BUILD_DIR)/gdb-git-04-source-highlight.done
	@mkdir -p $(GDB_DIR)-git/share/licenses/expat
	cp -p $(SOURCE_DIR_ABS)/$(EXPAT_SRC_DIR)/COPYING $(GDB_DIR)-git/share/licenses/expat/
	@mkdir -p $(GDB_DIR)-git/share/licenses/libiconv
	cp -p $(SOURCE_DIR_ABS)/$(ICONV_SRC_DIR)/COPYING.LIB $(GDB_DIR)-git/share/licenses/libiconv/
	@mkdir -p $(GDB_DIR)-git/share/licenses/boost
	cp -p $(SOURCE_DIR_ABS)/$(BOOST_SRC_DIR)/LICENSE_1_0.txt $(GDB_DIR)-git/share/licenses/boost/
	@mkdir -p $(GDB_DIR)-git/share/licenses/source-highlight
	cp -p $(SOURCE_DIR_ABS)/$(SOURCE_HIGHLIGHT_SRC_DIR)/COPYING $(GDB_DIR)-git/share/licenses/source-highlight/
	@mkdir -p $(GDB_DIR)-git/share/licenses/gmp
	cp -p $(SOURCE_DIR_ABS)/$(GMP_SRC_DIR)/COPYING $(GDB_DIR)-git/share/licenses/gmp
	@mkdir -p $(GDB_DIR)-git/share/licenses/mpfr
	cp -p $(SOURCE_DIR_ABS)/$(MPFR_SRC_DIR)/COPYING.LESSER $(GDB_DIR)-git/share/licenses/mpfr
	@mkdir -p $(GDB_DIR)-git/share/licenses/gdb
	cp -p $(GDB_GIT_DIR)/COPYING3 $(GDB_DIR)-git/share/licenses/gdb/
	@touch $@


# gdb-git-python

$(BUILD_DIR)/gdb-git-python-01-configure.done: | $(BUILD_DIR)/expat-05-make-install.done $(BUILD_DIR)/pdcurses-04-make-install.done $(BUILD_DIR)/iconv-05-make-install.done $(GDB_LIBS)/$(PYTHON_DIR) $(BUILD_DIR)/boost-03-regex.done $(BUILD_DIR)/source-highlight-05-make-install.done $(BUILD_DIR)/lzma-05-make-install.done $(BUILD_DIR)/gmp-05-make-install.done $(BUILD_DIR)/mpfr-05-make-install.done
	@mkdir -p $(BUILD_DIR)/gdb-git-python
	$(SET_PKG_PATH) cd $(BUILD_DIR)/gdb-git-python && $(GDB_GIT_CONF) --prefix=$(GDB_DIR)-git-python --with-python=$(GDB_LIBS)/$(PYTHON_DIR)/python
	@touch $@

$(BUILD_DIR)/gdb-git-python-02-make.done: | $(BUILD_DIR)/gdb-git-python-01-configure.done
	$(SET_PKG_PATH) $(MAKE) CC_FOR_BUILD=$(MYBUILD)-gcc -C $(BUILD_DIR)/gdb-git-python
	@touch $@

$(BUILD_DIR)/gdb-git-python-03-make-install.done: | $(BUILD_DIR)/gdb-git-python-02-make.done
	$(SET_PKG_PATH) $(MAKE) -C $(BUILD_DIR)/gdb-git-python/gdb install-strip
	$(SET_PKG_PATH) $(MAKE) -C $(BUILD_DIR)/gdb-git-python/gdbserver install-strip
	@touch $@

$(BUILD_DIR)/gdb-git-python-04-python.done: | $(BUILD_DIR)/gdb-git-python-03-make-install.done
	cp -af $(GDB_LIBS)/$(PYTHON_DIR)/python27.dll $(GDB_DIR)-git-python/bin/
	cp -arf $(GDB_LIBS)/$(PYTHON_DIR)/Lib $(GDB_DIR)-git-python/lib
	mkdir -p $(GDB_DIR)-git-python/DLLs
	cp -af $(GDB_LIBS)/$(PYTHON_DIR)/DLLs/_ctypes.pyd $(GDB_DIR)-git-python/DLLs
	rm -rf $(GDB_DIR)-git-python/lib/test
	@touch $@

$(BUILD_DIR)/gdb-git-python-05-source-highlight.done: | $(BUILD_DIR)/gdb-git-python-04-python.done
	cp -R $(GDB_LIBS)/share/source-highlight $(GDB_DIR)-git-python/share/source-highlight
	@touch $@

$(BUILD_DIR)/gdb-git-python-06-licenses.done: | $(BUILD_DIR)/gdb-git-python-05-source-highlight.done
	@mkdir -p $(GDB_DIR)-git-python/share/licenses/expat
	cp -p $(SOURCE_DIR_ABS)/$(EXPAT_SRC_DIR)/COPYING $(GDB_DIR)-git-python/share/licenses/expat/
	@mkdir -p $(GDB_DIR)-git-python/share/licenses/libiconv
	cp -p $(SOURCE_DIR_ABS)/$(ICONV_SRC_DIR)/COPYING.LIB $(GDB_DIR)-git-python/share/licenses/libiconv/
	@mkdir -p $(GDB_DIR)-git-python/share/licenses/python
	cp -p $(GDB_LIBS)/$(PYTHON_DIR)/LICENSE.txt $(GDB_DIR)-git-python/share/licenses/python/
	@mkdir -p $(GDB_DIR)-git-python/share/licenses/boost
	cp -p $(SOURCE_DIR_ABS)/$(BOOST_SRC_DIR)/LICENSE_1_0.txt $(GDB_DIR)-git-python/share/licenses/boost/
	@mkdir -p $(GDB_DIR)-git-python/share/licenses/source-highlight
	cp -p $(SOURCE_DIR_ABS)/$(SOURCE_HIGHLIGHT_SRC_DIR)/COPYING $(GDB_DIR)-git-python/share/licenses/source-highlight/
	@mkdir -p $(GDB_DIR)-git-python/share/licenses/gmp
	cp -p $(SOURCE_DIR_ABS)/$(GMP_SRC_DIR)/COPYING $(GDB_DIR)-git-python/share/licenses/gmp
	@mkdir -p $(GDB_DIR)-git-python/share/licenses/mpfr
	cp -p $(SOURCE_DIR_ABS)/$(MPFR_SRC_DIR)/COPYING.LESSER $(GDB_DIR)-git-python/share/licenses/mpfr
	@mkdir -p $(GDB_DIR)-git-python/share/licenses/gdb
	cp -p $(GDB_GIT_DIR)/COPYING3 $(GDB_DIR)-git-python/share/licenses/gdb/
	@touch $@


# binutils-git

$(BUILD_DIR)/binutils-git-01-configure.done:
	@mkdir -p $(BUILD_DIR)/binutils-git
	cd $(BUILD_DIR)/binutils-git && $(BINUTILS_GIT_CONF)
	@touch $@

$(BUILD_DIR)/binutils-git-02-make.done: | $(BUILD_DIR)/binutils-git-01-configure.done
	$(MAKE) -C $(BUILD_DIR)/binutils-git
	@touch $@


# gdb-testsuite

$(BUILD_DIR)/gdb-test-01-configure.done: | $(BUILD_DIR)/expat-05-make-install.done $(BUILD_DIR)/pdcurses-04-make-install.done $(BUILD_DIR)/iconv-05-make-install.done $(BUILD_DIR)/boost-03-regex.done $(BUILD_DIR)/source-highlight-05-make-install.done
	@mkdir -p $(BUILD_DIR)/gdb-test
	$(SET_PKG_PATH) cd $(BUILD_DIR)/gdb-test && $(GDB_TEST_CONF) --prefix=$(GDB_DIR)-test --with-python=$(GDB_LIBS)/$(PYTHON_DIR)/python
	@touch $@

$(BUILD_DIR)/gdb-test-02-make.done: | $(BUILD_DIR)/gdb-test-01-configure.done
	$(SET_PKG_PATH) $(MAKE) CC_FOR_BUILD=$(MYBUILD)-gcc -C $(BUILD_DIR)/gdb-test
	@touch $@


# redhat64-gdb

$(BUILD_DIR)/gdb-redhat64-01-configure.done: | $(BUILD_DIR)/expat-05-make-install.done $(BUILD_DIR)/pdcurses-04-make-install.done $(BUILD_DIR)/iconv-05-make-install.done $(BUILD_DIR)/boost-03-regex.done $(BUILD_DIR)/source-highlight-05-make-install.done
	@mkdir -p $(BUILD_DIR)/gdb-redhat64
	$(SET_PKG_PATH) cd $(BUILD_DIR)/gdb-redhat64 && $(GDB_REDHAT64_CONF) --prefix=$(GDB_DIR)-redhat64
	@touch $@

$(BUILD_DIR)/gdb-redhat64-02-make.done: | $(BUILD_DIR)/gdb-redhat64-01-configure.done
	$(SET_PKG_PATH) $(MAKE) CC_FOR_BUILD=$(MYBUILD)-gcc -C $(BUILD_DIR)/gdb-redhat64
	@touch $@


# redhat32-host-gdb

$(BUILD_DIR)/gdb-redhat32-host-01-configure.done: | $(BUILD_DIR)/expat-05-make-install.done $(BUILD_DIR)/pdcurses-04-make-install.done $(BUILD_DIR)/iconv-05-make-install.done $(BUILD_DIR)/boost-03-regex.done $(BUILD_DIR)/source-highlight-05-make-install.done
	@mkdir -p $(BUILD_DIR)/gdb-redhat32-host
	$(SET_PKG_PATH) cd $(BUILD_DIR)/gdb-redhat32-host && $(GDB_REDHAT32_HOST_CONF) --prefix=$(GDB_DIR)-redhat32-host
	@touch $@

$(BUILD_DIR)/gdb-redhat32-host-02-make.done: | $(BUILD_DIR)/gdb-redhat32-host-01-configure.done
	$(SET_PKG_PATH) $(MAKE) CC_FOR_BUILD=$(MYBUILD)-gcc -C $(BUILD_DIR)/gdb-redhat32-host
	@touch $@


extract-all: | \
  $(SOURCE_DIR)/expat-01-extract.done \
  $(SOURCE_DIR)/pdcurses-01-extract.done \
  $(SOURCE_DIR)/iconv-01-extract.done \
  $(SOURCE_DIR)/boost-01-extract.done \
  $(SOURCE_DIR)/source-highlight-01-extract.done \
  $(SOURCE_DIR)/lzma-01-extract.done \
  $(SOURCE_DIR)/gmp-01-extract.done \
  $(SOURCE_DIR)/mpfr-01-extract.done \


patch-all: | \
  $(SOURCE_DIR)/pdcurses-02-patch-13-ncurses-mouse-api.done \
  $(SOURCE_DIR)/source-highlight-02-patch-02-remove-throw.done \


build-expat: | $(BUILD_DIR)/expat-05-make-install.done
build-pdcurses: | $(BUILD_DIR)/pdcurses-04-make-install.done
build-iconv: | $(BUILD_DIR)/iconv-05-make-install.done
build-boost: | $(BUILD_DIR)/boost-03-regex.done
build-source-highlight: | $(BUILD_DIR)/source-highlight-05-make-install.done
build-lzma: | $(BUILD_DIR)/lzma-05-make-install.done
build-gmp: | $(BUILD_DIR)/gmp-05-make-install.done
build-mpfr: | $(BUILD_DIR)/mpfr-05-make-install.done
build-gdb: | $(BUILD_DIR)/gdb-git-05-licenses.done
build-gdb-python: | $(BUILD_DIR)/gdb-git-python-06-licenses.done
build-binutils: | $(BUILD_DIR)/binutils-git-02-make.done


gdb$(BUILD_BITS).7z: | build-gdb
	@rm -f $@
	cd $(GDB_DIR)-git && 7z a -mx=9 ../$@ *

gdb$(BUILD_BITS)-python.7z: | build-gdb-python
	@rm -f $@
	cd $(GDB_DIR)-git-python && 7z a -mx=9 ../$@ *


package-gdb: gdb$(BUILD_BITS).7z
package-gdb-python: gdb$(BUILD_BITS)-python.7z

ifeq ($(BUILD_BITS),32)
gdb64.7z gdb64-python.7z:
	$(MAKE) BUILD_BITS=64 $@
else
gdb32.7z gdb32-python.7z:
	$(MAKE) BUILD_BITS=32 $@
endif

packages: gdb32.7z
packages: gdb32-python.7z
packages: gdb64.7z
packages: gdb64-python.7z


info:
	@echo -e "\r"
	@echo -e "$(EXPAT_FILE)\r"
	@echo -e "$(PDCURSES_FILE)\r"
	@echo -e "$(ICONV_FILE)\r"
	@echo -e "$(PYTHON_FILE)\r"
	@echo -e "$(GDB_FILE)\r"
