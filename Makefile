CXXFLAGS = -std=c++11 -pthread -Wall -pedantic -Wpedantic -march=native -Wl,--no-as-needed -g3 

TFLAGS = -O3
TFLAGS+= -finline 
TFLAGS+= -march=native
TFLAGS+= -fmerge-all-constants
TFLAGS+= -fmodulo-sched
TFLAGS+= -fmodulo-sched-allow-regmoves
TFLAGS+= -fsched-pressure
TFLAGS+= -fipa-pta
TFLAGS+= -fipa-matrix-reorg
TFLAGS+= -ftree-loop-distribution
TFLAGS+= -ftracer
TFLAGS+= -funroll-loops
# TFLAGS+= -fwhole-program
# TFLAGS+= -flto
TFLAGS+= -DNDEBUG
TFLAGS+= -DPARALLELIMPL=true

LDFLAGS =  -lnsl 

SRC_ALL = Board/BitBoard.cpp Board/BitUtils.cpp Utils.cpp
OBJ_ALL = $(SRC_ALL:.cpp=.o)

# SRC_SER = Communication/Comm.hpp Server.cpp 
SRC_SER =  Communication/Comm.cpp Server.cpp 
OBJ_SER = $(SRC_SER:.cpp=.o)

SRC_CLN = Communication/Client-comm.cpp Client.cpp NegaScout/Search.cpp
OBJ_CLN = $(SRC_CLN:.cpp=.o)

PAR_DIR=parallel
SIN_DIR=single

BIN_ROOT=bin
OBJ_ROOT=obj
DEP_ROOT=depend

BIN_PAR=$(BIN_ROOT)/$(PAR_DIR)/
BIN_SIN=$(BIN_ROOT)/$(SIN_DIR)/

OBJ_PAR=$(OBJ_ROOT)/$(PAR_DIR)/
OBJ_SIN=$(OBJ_ROOT)/$(SIN_DIR)/

DEP_PAR=$(DEP_ROOT)/$(PAR_DIR)/
DEP_SIN=$(DEP_ROOT)/$(SIN_DIR)/

SED_OPD=$(subst /,\/,$(OBJ_PAR))
SED_OSD=$(subst /,\/,$(OBJ_SIN))

SED_DPD=$(subst /,\/,$(DEP_PAR))
SED_DSD=$(subst /,\/,$(DEP_SIN))

CXX_SOURCES = $(SRC_SER) $(SRC_CLN) $(SRC_ALL)

release:CXXFLAGS += $(TFLAGS)

CXXFLAGS += $(LDFLAGS)

parallel:PS_IMP:= -DPARALLELIMPL=true
parallel:BIN_DIR:= $(BIN_PAR)
parallel:IMP_DIR:= $(PAR_DIR)
parallel:OBJ_DIR:= $(OBJ_PAR)
parallel:OBJ_ALL_D:= $(addprefix $(OBJ_PAR), $(OBJ_ALL))
parallel:OBJ_SER_D:= $(addprefix $(OBJ_PAR), $(OBJ_SER))
parallel:OBJ_CLN_D:= $(addprefix $(OBJ_PAR), $(OBJ_CLN))

single:PS_IMP:= -DPARALLELIMPL=false
single:BIN_DIR:= $(BIN_SIN)
single:IMP_DIR:= $(SIN_DIR)
single:OBJ_DIR:= $(OBJ_SIN)
single:OBJ_ALL_D:= $(addprefix $(OBJ_SIN), $(OBJ_ALL))
single:OBJ_SER_D:= $(addprefix $(OBJ_SIN), $(OBJ_SER))
single:OBJ_CLN_D:= $(addprefix $(OBJ_SIN), $(OBJ_CLN))

$(BIN_PAR)server:$(addprefix $(OBJ_PAR), $(OBJ_ALL) $(OBJ_SER))
$(BIN_PAR)client:$(addprefix $(OBJ_PAR), $(OBJ_ALL) $(OBJ_CLN))
$(BIN_SIN)server:$(addprefix $(OBJ_SIN), $(OBJ_ALL) $(OBJ_SER))
$(BIN_SIN)client:$(addprefix $(OBJ_SIN), $(OBJ_ALL) $(OBJ_CLN))

OBJ_FILES:=$(addprefix $(OBJ_PAR), $(OBJ_ALL) $(OBJ_SER) $(OBJ_CLN)) $(addprefix $(OBJ_SIN), $(OBJ_ALL) $(OBJ_SER) $(OBJ_CLN))


space= 
#do no remove this lines!!! needed!!!
space+= 

vpath %.o $(subst $(space),:,$(dir $(OBJ_FILES)))
vpath %.cpp $(subst $(space),:,$(dir $(CXX_SOURCES)))

parallel:$(BIN_PAR)server $(BIN_PAR)client
single:$(BIN_SIN)server $(BIN_SIN)client

.PHONY: all parallel single release debug

all: release
release:clean | parallel single
debug:clean | parallel single

%.o: 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(PS_IMP) -c $(subst $(OBJ_DIR),,$(@:.o=.cpp)) -o $@

clean:
	-rm -r $(OBJ_ROOT) $(BIN_ROOT) $(DEP_ROOT)
	-rm $(PAR_DIR)_server $(PAR_DIR)_client $(SIN_DIR)_server $(SIN_DIR)_client
	mkdir -p $(BIN_PAR) $(BIN_SIN) $(OBJ_PAR) $(OBJ_SIN) $(DEP_PAR) $(DEP_SIN)
	mkdir -p $(dir $(OBJ_FILES))

%server:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(PS_IMP) -o $@ $^
	-@rm $(IMP_DIR)_server
	ln -rs $@ $(IMP_DIR)_server

%client:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(PS_IMP) -o $@ $^
	-@rm $(IMP_DIR)_client
	ln -rs $@ $(IMP_DIR)_client

$(DEP_PAR)%.d: %.cpp
	@mkdir -p $(@D)
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) -DPARALLELIMPL=true  $< | sed -r 's/^(\S+).(\S+):/$(SED_OPD)$(subst /,\/,$(<:.cpp=.o)) $(SED_DPD)$(subst /,\/,$(<:.cpp=.d)): \\\n/g' > $@

$(DEP_SIN)%.d: %.cpp
	@mkdir -p $(@D)
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) -DPARALLELIMPL=false $< | sed -r 's/^(\S+).(\S+):/$(SED_OSD)$(subst /,\/,$(<:.cpp=.o)) $(SED_DSD)$(subst /,\/,$(<:.cpp=.d)): \\\n/g' > $@

-include $(addprefix $(DEP_PAR), $(CXX_SOURCES:.cpp=.d))
-include $(addprefix $(DEP_SIN), $(CXX_SOURCES:.cpp=.d))
