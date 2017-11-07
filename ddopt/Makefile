# --- USER COMPILATION SETTINGS --

# Debug mode
DEBUG_MODE = 0

# Use CPLEX; used almost everywhere
USE_CPLEX = 1

# Set CPLEX directory as follows:
# BASEILOGDIR = /opt/ibm/ILOG/CPLEX_Enterprise_Server126/CPLEX_Studio

# Use GMP library; used to compute the center of a decision diagram (not necessary for reproducing experiments)
USE_GMP = 0

# Use ConicBundle; used in Lagrangian relaxation (not necessary for reproducing experiments)
USE_CONICBUNDLE = 0
CONICBUNDLEDIR = ConicBundle


# --- SYSTEM ---

SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic


# --- CPLEX DIRECTORIES ---

CONCERTDIR  = $(BASEILOGDIR)/concert
CPLEXDIR    = $(BASEILOGDIR)/cplex


# --- CPLEX FLAGS ---

CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include


# --- OPTIMIZATION FLAGS ---

# These are named USERCFLAGS and USERLDFLAGS because SCIP defines its own CFLAGS and LDFLAGS
USERCFLAGS = -ansi -DIL_STD -fPIC -fstrict-aliasing -pedantic -Wall -Wextra -Wno-long-long -Wno-unused-parameter -fexceptions \
	-ffloat-store -std=c++11 -I./boost $(DEBUG_OPT) -c
USERLDFLAGS = -lm -pthread

ifeq ($(USE_GMP),1)
USERCFLAGS += -DUSE_GMP
USERLDFLAGS += -lgmpxx -lgmp
endif

ifeq ($(USE_CONICBUNDLE),1)
USERCFLAGS += -DUSE_CONICBUNDLE -I$(CONICBUNDLEDIR)/include
USERLDFLAGS += -L$(CONICBUNDLEDIR)/lib -lcb
endif


# --- SOLVER-DEPENDENT SETUP ---

# CPLEX setup
ifeq ($(USE_CPLEX),1)
USERCFLAGS += -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) -DSOLVER_CPLEX -c
USERLDFLAGS += -L$(CPLEXLIBDIR) -lilocplex -lcplex -L$(CONCERTLIBDIR) -lconcert
endif

# Files that require CPLEX
CPLEX_FILES = $(shell find src/ -name '*_cplex.cpp')
ifneq ($(USE_CPLEX),1)
EXCLUDE = $(CPLEX_FILES)
endif


# --- COMPILER ---

CCC = g++


# --- DEBUG FLAGS ---

ifeq ($(DEBUG_MODE),1)
DEBUG_OPT = -g -O0
else
DEBUG_OPT = -DNDEBUG -O3
endif


# --- DIRECTORIES  ---

SRC_DIR   := src
OBJ_DIR   := obj

SRC_DIRS  := $(shell find $(SRC_DIR) -type d)
OBJ_DIRS  := $(addprefix $(OBJ_DIR)/,$(SRC_DIRS))

SOURCES   := $(shell find $(SRC_DIR) -name '*.cpp')
SOURCES   := $(filter-out $(EXCLUDE), $(SOURCES))
OBJ_FILES := $(addprefix $(OBJ_DIR)/, $(SOURCES:.cpp=.o))

vpath %.cpp $(SRC_DIRS)


# --- TARGETS ---

EXECUTABLE=ddopt

all: $(EXECUTABLE)
	cp $(EXECUTABLE) ../tests/bin

$(EXECUTABLE): testboostdir testcplexdir makedir $(SOURCES) $(OBJ_FILES)
	$(CCC) $(OBJ_FILES) $(USERLDFLAGS) -o $@

$(OBJ_DIR)/%.o: %.cpp
	$(CCC) $(FLAGS) $(OFLAGS) $(BINOFLAGS) $(CXXFLAGS) $(USERCFLAGS) $< -o $@

makedir: $(OBJ_DIRS)

testcplexdir:
	@if [ "$(USE_CPLEX)" = "1" -a "$(BASEILOGDIR)" = "" ]; then echo "*** Error: CPLEX directory is not set; modify Makefile"; exit 1; fi

testboostdir:
	@if [ ! -d "boost" ]; then echo "Unpacking boost library..."; tar -xzf deps/boost_deps.tar.gz; fi

$(OBJ_DIRS):
	@mkdir -p $@

clean:
	@rm -rf obj 
	@rm -f $(EXECUTABLE)
