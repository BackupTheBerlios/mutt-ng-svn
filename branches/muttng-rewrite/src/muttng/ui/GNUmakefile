FILES=ui ui_curses ui_plain ui_text \
      abstract_screen config_screen plain_config_screen curses_config_screen \
      abstract_screen_motion plain_screen_motion curses_screen_motion curses_screen_page_motion curses_screen_line_motion
LIB=libui.a

all: $(LIB)

include $(CURDIR)/../../../GNUmakefile.config.default

CXXFLAGS+=-I.. -I../..

include $(CURDIR)/../../../GNUmakefile.compile_cpp
