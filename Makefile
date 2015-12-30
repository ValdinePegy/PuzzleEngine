# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -std=c++11 -I../Empirical/ -I./

# Native compiler information
CXX_nat := g++-4.8
#CFLAGS_nat := -g $(CFLAGS_all)    # Debug mode
#CFLAGS_nat := -O3 $(CFLAGS_all)   # Optimized mode
CFLAGS_nat := $(CFLAGS_all) -pg   # Profile mode

# Emscripten compiler information
CXX_web := emcc
#OFLAGS_web := -g4 -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 # -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
OFLAGS_web := -DNDEBUG -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Oz -DNDEBUG -s TOTAL_MEMORY=67108864 -s ASSERTIONS=1
#OFLAGS_web := -Os -DNDEBUG -s TOTAL_MEMORY=67108864


CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../Empirical/emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s COMPILER_ASSERTIONS=1 -s NO_EXIT_RUNTIME=1 --embed-file configs
default: PuzzleEngine
web: PuzzleEngine.js
all: PuzzleEngine PuzzleEngine.js

SRC	:= source/Sudoku.cc

PuzzleEngine:	$(SRC) source/drivers/command_line.cc
	$(CXX_nat) $(CFLAGS_nat) $(SRC) source/drivers/command_line.cc -o PuzzleEngine
	@echo To build the web version use: make web

PuzzleEngine.js: $(SRC) source/drivers/html.cc
	$(CXX_web) $(CFLAGS_web) $(SRC) source/drivers/html.cc -o PuzzleEngine.js

clean:
	rm -f PuzzleEngine PuzzleEngine.js *.js.map *~ source/*.o source/*/*.o

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
