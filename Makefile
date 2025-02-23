###############################################################################
# Top-Level Makefile for Building Both "cgi_id" and "idpcdu"
#
# This Makefile assumes:
#   - "cgi_id/" has only .c/.h files for building cgi_id.exe
#   - "idpcd2/" has .cpp and some .c files for building idpcdu
#   - "nic_lib/" has some .cpp files needed by idpcdu
#   - We place all build artifacts into the "build/" directory
#   - We keep old-style compiler flags that were used historically
#
# Basic usage:
#   make            # Builds everything (cgi_id.exe + idpcdu)
#   make cgi_id     # Only build cgi_id.exe
#   make idpcdu     # Only build idpcdu
#   make clean      # Clean all build artifacts
###############################################################################

# --------------------------------------------------------------------------
# Tools and Flags (customize if needed)
# --------------------------------------------------------------------------

# Compiler for old C-based code (cgi_id)
CC      := gcc
CFLAGS  :=-DUNIX  # minimal flags, as per 2000-era code

# Compiler for old C++-based daemon (idpcdu). Historically it also compiled .c
CXX     := g++
#CXXFLAGS := -Wall -pedantic -DUNIX -I nic_lib  -Wno-write-strings -fpermissive # include nic_lib for .h
CXXFLAGS := -Wall -pedantic -DUNIX -I nic_lib  -Wno-write-strings -fpermissive # include nic_lib for .h

# (If you need libraries, link them here.)
LIBS_CGI   := -lm
LIBS_IDPCDU :=

# Output directory for build artifacts
OUTDIR := build

# --------------------------------------------------------------------------
# cgi_id Sources
# (All .c files from the old cgi_id makefile.)
# --------------------------------------------------------------------------
CGI_ID_SOURCES := \
  cgi_id/db_bugbase.c \
  cgi_id/bugrep_actions.c \
  cgi_id/cgi_id.c \
  cgi_id/page_doc_request.c \
  cgi_id/cgic.c \
  cgi_id/connection_zone.c \
  cgi_id/file_io_subsystem.c \
  cgi_id/html_subsystem.c \
  cgi_id/message_subsystem.c \
  cgi_id/net_subsystem.c \
  cgi_id/page_add_bug_result.c \
  cgi_id/page_add_user.c \
  cgi_id/users.c \
  cgi_id/rank_calc.c \
  cgi_id/page_mass_user_options.c \
  cgi_id/page_add_user_result.c \
  cgi_id/page_daemon_stats.c \
  cgi_id/page_delete_user.c \
  cgi_id/page_edit_bug.c \
  cgi_id/page_edit_user.c \
  cgi_id/page_edit_user_result.c \
  cgi_id/page_elements.c \
  cgi_id/page_find_bug.c \
  cgi_id/page_list_bugs.c \
  cgi_id/page_user_listing.c \
  cgi_id/page_web_stats.c \
  cgi_id/page_main_menu.c \
  cgi_id/page_authenticate.c \
  cgi_id/page_bug_repository.c \
  cgi_id/page_add_bug.c \
  cgi_id/page_sysset.c \
  cgi_id/page_download.c \
  cgi_id/page_download_reg.c \
  cgi_id/page_homepage.c \
  cgi_id/page_invalid_request.c \
  cgi_id/page_news.c \
  cgi_id/page_faq.c \
  cgi_id/page_order.c \
  cgi_id/page_news_admin.c \
  cgi_id/page_faq_admin.c \
  cgi_id/page_ranking.c \
  cgi_id/page_support.c \
  cgi_id/port_subsystem.c \
  cgi_id/srvstat_actions.c \
  cgi_id/db_system_settings.c \
  cgi_id/db_news_articles.c \
  cgi_id/db_faq_entries.c \
  cgi_id/database_subsystem.c \
  cgi_id/license_manager.c \
  cgi_id/mime64.c \
  cgi_id/mail_encode.c

# Object files go under build/cgi_id/<filename>.o
CGI_ID_OBJECTS := $(patsubst cgi_id/%.c,$(OUTDIR)/cgi_id/%.o,$(CGI_ID_SOURCES))


# --------------------------------------------------------------------------
# idpcdu Sources
# (All .cpp and .c from the old idpcd2 makefile, plus nic_lib references.)
# Note: The old idpcd2 makefile uses g++ even for .c files, so we'll do that.
# --------------------------------------------------------------------------
IDPCDU_CPP_SOURCES := \
  idpcd2/main.cpp \
  idpcd2/command_line.cpp \
  idpcd2/connection.cpp \
  idpcd2/connection_idpcd.cpp \
  idpcd2/connection_pool.cpp \
  idpcd2/group_pool.cpp \
  idpcd2/idpcd_group.cpp \
  idpcd2/idpcd_group_messages.cpp \
  idpcd2/net_messages_x.cpp \
  idpcd2/startup_daemon.cpp \
  idpcd2/thread_listening.cpp \
  idpcd2/thread_processing.cpp \
  idpcd2/unique_id.cpp

IDPCDU_C_SOURCES := \
  idpcd2/database_subsystem.c \
  idpcd2/file_io_subsystem.c \
  idpcd2/port_subsystem.c \
  idpcd2/net_subsystem.c \
  idpcd2/users.c \
  idpcd2/rank_calc.c

# nic_lib references
NIC_LIB_SOURCES := \
  nic_lib/common.cpp \
  nic_lib/result.cpp \
  nic_lib/standard_config_file.cpp

# Combine them:
IDPCDU_ALL_SOURCES := $(IDPCDU_CPP_SOURCES) $(IDPCDU_C_SOURCES) $(NIC_LIB_SOURCES)

# Object files go under build/idpcd2/<filename>.o
IDPCDU_OBJECTS := \
  $(patsubst idpcd2/%.cpp,$(OUTDIR)/idpcd2/%.o,$(IDPCDU_CPP_SOURCES)) \
  $(patsubst idpcd2/%.c,$(OUTDIR)/idpcd2/%.o,$(IDPCDU_C_SOURCES)) \
  $(patsubst nic_lib/%.cpp,$(OUTDIR)/idpcd2/nic_lib/%.o,$(NIC_LIB_SOURCES))

# --------------------------------------------------------------------------
# Final Targets
# --------------------------------------------------------------------------

.PHONY: all cgi_id idpcdu clean strip

# "all" builds both
all: cgi_id idpcdu

# Build cgi_id (an .exe by tradition)
cgi_id: $(OUTDIR)/cgi_id.exe
	@echo "[ OK ] Finished building cgi_id -> $@"

$(OUTDIR)/cgi_id.exe: $(CGI_ID_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS_CGI)

# Build idpcdu (no .exe extension typically on UNIX)
idpcdu: $(OUTDIR)/idpcdu
	@echo "[ OK ] Finished building idpcdu -> $@"

$(OUTDIR)/idpcdu: $(IDPCDU_OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS_IDPCDU)

# --------------------------------------------------------------------------
# Compilation Rules (pattern rules)
# --------------------------------------------------------------------------

# For cgi_id: .c -> build/cgi_id/*.o using "gcc"
$(OUTDIR)/cgi_id/%.o: cgi_id/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# For idpcdu (and nic_lib) .cpp -> build/idpcd2/... using "g++"
$(OUTDIR)/idpcd2/%.o: idpcd2/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUTDIR)/idpcd2/nic_lib/%.o: nic_lib/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# For idpcdu's .c -> also compiled by "g++" (per original idpcd2 makefile).
$(OUTDIR)/idpcd2/%.o: idpcd2/%.c
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# --------------------------------------------------------------------------
# clean / strip / etc.
# --------------------------------------------------------------------------

clean:
	@echo "Cleaning all build artifacts..."
	rm -rf $(OUTDIR)
	@echo "Done cleaning!"

# Simple "strip" that removes carriage returns (\r) from source files
# (An old trick from original Makefiles.)
strip:
	@echo "Stripping DOS line endings from source files..."
	for f in $$(find cgi_id idpcd2 nic_lib -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' \)); do \
	  tr -d '\r' < $$f > tmp && mv tmp $$f; \
	done
	@rm -f tmp
	@echo "Done stripping!"

