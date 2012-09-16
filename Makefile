CXX      ?= g++
AR       ?= ar
CXXFLAGS ?= -O2 -g3 -Werror
CXXFLAGS += -std=c++11 -Wall

CXXFLAGS += $(shell sdl-config --cflags)

PANICFIRELIBS = $(shell sdl-config --libs) -lSDL_image -lSDL_ttf -lGL -lboost_serialization -lboost_iostreams

CXXFLAGS += -Isrc
BINDIR       = bin

# Common lib

COMMONSRCDIR = src/common
COMMONLIB = $(COMMONSRCDIR)/libcommon.a

# Panicfire

PANICFIREBINNAME = panicfire
PANICFIREBIN     = $(BINDIR)/$(PANICFIREBINNAME)
PANICFIRESRCDIR = src/panicfire
PANICFIRESRCFILES = game/Game.cpp main.cpp

PANICFIRESRCS = $(addprefix $(PANICFIRESRCDIR)/, $(PANICFIRESRCFILES))
PANICFIREOBJS = $(PANICFIRESRCS:.cpp=.o)
PANICFIREDEPS = $(PANICFIRESRCS:.cpp=.dep)


.PHONY: clean all

all: $(PANICFIREBIN)

$(BINDIR):
	mkdir -p $(BINDIR)

COMMONDIR = src/common
COMMONSRCS = $(shell (find $(COMMONDIR) \( -name '*.cpp' -o -name '*.h' \)))

$(COMMONLIB): $(COMMONSRCS)
	make -C $(COMMONDIR)

$(PANICFIREBIN): $(COMMONLIB) $(PANICFIREOBJS) $(BINDIR)
	$(CXX) $(LDFLAGS) $(PANICFIRELIBS) $(PANICFIREOBJS) $(COMMONLIB) -o $(PANICFIREBIN)

%.dep: %.cpp
	@rm -f $@
	@$(CC) -MM $(CXXFLAGS) $< > $@.P
	@sed 's,\($(notdir $*)\)\.o[ :]*,$(dir $*)\1.o $@ : ,g' < $@.P > $@
	@rm -f $@.P

clean:
	find src/ -name '*.o' -exec rm -rf {} +
	find src/ -name '*.dep' -exec rm -rf {} +
	find src/ -name '*.a' -exec rm -rf {} +
	rm -rf $(PANICFIREBIN)
	rmdir $(BINDIR)

-include $(PANICFIREDEPS)

