CC    := gcc
CXX   := g++
UNAME := $(shell uname -s)

OPTIMIZATION := -O3 -fPIC -DNDEBUG
WARNINGS := -Wall -Wunreachable-code -Wfloat-equal -Wredundant-decls -Winit-self -Wpedantic
CFLAGS   := $(WARNINGS) $(OPTIMIZATION) -std=c99
CXXFLAGS := $(WARNINGS) $(OPTIMIZATION) -std=c++11
LDFLAGS  := -shared -fPIC

OBJDIR := build
ifeq ($(UNAME),Darwin)
LIB_EXT :=.dylib
else
LIB_EXT :=.so
endif

CPPSOURCES  := $(wildcard bad-mutex/*.cpp)
CSOURCES    := $(wildcard bad-mutex/*.c)
BM_OBJECTS  := $(addprefix $(OBJDIR)/, $(CPPSOURCES:.cpp=.o) $(CSOURCES:.c=.o))
BadMutex    := $(OBJDIR)/libBadMutex$(LIB_EXT)
BadMutexLua := $(OBJDIR)/BadMutex.lua

CPPSOURCES  := $(wildcard threaded-libcurl/*.cpp)
CSOURCES    := $(wildcard threaded-libcurl/*.c)
DM_OBJECTS  := $(addprefix $(OBJDIR)/, $(CPPSOURCES:.cpp=.o) $(CSOURCES:.c=.o))
DownloadManager := $(OBJDIR)/libDownloadManager$(LIB_EXT)
DownloadManagerLua := $(OBJDIR)/DownloadManager.lua

CPPSOURCES := $(wildcard precise-timer/*.cpp)
CSOURCES   := $(wildcard precise-timer/*.c)
PT_OBJECTS := $(addprefix $(OBJDIR)/, $(CPPSOURCES:.cpp=.o) $(CSOURCES:.c=.o))
PreciseTimer := $(OBJDIR)/libPreciseTimer$(LIB_EXT)
PreciseTimerLua := $(OBJDIR)/PreciseTimer.lua

RequireFFILua := $(OBJDIR)/requireffi.lua

.PHONY: all debug BadMutex DownloadManager PreciseTimer lua clean
all: BadMutex DownloadManager PreciseTimer

debug: CFLAGS += -g -O1 -UNDEBUG
debug: CXXFLAGS += -g -O1 -UNDEBUG
debug: LDFLAGS += -g
debug: all

BadMutex: $(BadMutex)
DownloadManager: $(DownloadManager)
PreciseTimer: $(PreciseTimer)

$(BM_OBJECTS): | $(OBJDIR)/bad-mutex/
$(DM_OBJECTS): | $(OBJDIR)/threaded-libcurl/
$(PT_OBJECTS): | $(OBJDIR)/precise-timer/

$(BadMutex): $(BM_OBJECTS)
	@printf "\e[1;32m LINK\e[m $@\n"
	@$(CXX) $^ $(LDFLAGS) -o $@

$(DownloadManager): LDFLAGS += -lcurl
ifeq ($(UNAME),Darwin)
$(DownloadManager): LDFLAGS += -framework CoreFoundation -framework SystemConfiguration
else
# std::thread won't work without explicitly linking pthreads on linux.
$(DownloadManager): LDFLAGS += -pthread
endif
$(DownloadManager): $(DM_OBJECTS)
	@printf "\e[1;32m LINK\e[m $@\n"
	@$(CXX) $(LDFLAGS) -o $@ $^

$(PreciseTimer): $(PT_OBJECTS)
	@printf "\e[1;32m LINK\e[m $@\n"
	@$(CXX) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.c
	@printf "\e[1;34m   CC\e[m $<\n"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.cpp
	@printf "\e[1;34m  CXX\e[m $<\n"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	@printf "\e[1;33mMKDIR\e[m $@\n"
	@mkdir -p $@

$(OBJDIR)/%/: | $(OBJDIR)
	@printf "\e[1;33mMKDIR\e[m $@\n"
	@mkdir -p $@

$(BadMutexLua): bad-mutex/BadMutex.moon | $(OBJDIR)
	@printf "\e[1;35mMOONC\e[m $@\n"
	$(shell ./BuildLua.sh bad-mutex BadMutex $@)

$(DownloadManagerLua): threaded-libcurl/DownloadManager.moon | $(OBJDIR)
	@printf "\e[1;35mMOONC\e[m $@\n"
	$(shell ./BuildLua.sh threaded-libcurl DownloadManager $@)

$(PreciseTimerLua): precise-timer/PreciseTimer.moon | $(OBJDIR)
	@printf "\e[1;35mMOONC\e[m $@\n"
	$(shell ./BuildLua.sh precise-timer PreciseTimer $@)

$(RequireFFILua): | $(OBJDIR)
	@printf "\e[1;35mMOONC\e[m $@\n"
	$(shell moonc -o $@ requireffi/requireffi.moon 2> /dev/null)

lua: $(BadMutexLua) $(DownloadManagerLua) $(PreciseTimerLua) $(RequireFFILua)

clean:
	@printf "\e[1;31m   RM\e[m $(OBJDIR)\n"
	@rm -rf $(OBJDIR)

