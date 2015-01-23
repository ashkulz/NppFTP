# Makefile for building NppFTP
CXX    = i686-w64-mingw32-g++
AR     = ar rcs
CFLAGS = -MMD -Os -O3 -Wall -Werror -fexpensive-optimizations -DLIBSSH_STATIC -DUNICODE -D_UNICODE
LFLAGS = -static -Lobj -L3rdparty/lib -lcomdlg32 -lcomctl32 -luuid -lole32 -lshlwapi -lssh -lssl -lcrypto -lz -lgdi32 -lws2_32
INC    = -I3rdparty/include -Iinclude -Iinclude/Npp -Iinclude/Windows -Itinyxml/include -IUTCP/include
RES    = obj/NppFTP.res

TGT    = bin/NppFTP.dll
TXML_OBJ = $(patsubst tinyxml/src/%.cpp,obj/%.o,$(wildcard tinyxml/src/*.cpp))
UTCP_OBJ = $(patsubst UTCP/src/%.cpp,obj/%.o,$(wildcard UTCP/src/*.cpp))
NPP_OBJ  = $(patsubst src/%.cpp,obj/%.o,$(wildcard src/*.cpp)) $(patsubst src/Windows/%.cpp,obj/%.o,$(wildcard src/Windows/*.cpp))
OBJECTS  = $(TXML_OBJ) $(UTCP_OBJ) $(NPP_OBJ)
DEPENDS  = ${OBJECTS:.o=.d}

TGT_D      = bin/NppFTPd.dll
TXML_OBJ_D = $(patsubst tinyxml/src/%.cpp,obj/%_debug.o,$(wildcard tinyxml/src/*.cpp))
UTCP_OBJ_D = $(patsubst UTCP/src/%.cpp,obj/%_debug.o,$(wildcard UTCP/src/*.cpp))
NPP_OBJ_D  = $(patsubst src/%.cpp,obj/%_debug.o,$(wildcard src/*.cpp)) $(patsubst src/Windows/%.cpp,obj/%_debug.o,$(wildcard src/Windows/*.cpp))
OBJECTS_D  = $(TXML_OBJ_D) $(UTCP_OBJ_D) $(NPP_OBJ_D)
DEPENDS_D  = ${OBJECTS_D:.o=.d}

all:     release
debug:   makedirs $(TGT_D)
release: makedirs $(TGT)
	@zip -9 -r NppFTP.zip $(TGT) doc/ >nul

obj/%.o: tinyxml/src/%.cpp
	@echo CXX  $< & $(CXX) -c $(CFLAGS) $(INC) $< -o $@

obj/%.o: UTCP/src/%.cpp
	@echo CXX  $< & $(CXX) -c $(CFLAGS) $(INC) $< -o $@

obj/%.o: src/%.cpp
	@echo CXX  $< & $(CXX) -c $(CFLAGS) $(INC) $< -o $@

obj/%.o: src/Windows/%.cpp
	@echo CXX  $< & $(CXX) -c $(CFLAGS) $(INC) $< -o $@

obj/%_debug.o: tinyxml/src/%.cpp
	@echo CXX  $< & $(CXX) -g -c $(CFLAGS) $(INC) $< -o $@

obj/%_debug.o: UTCP/src/%.cpp
	@echo CXX  $< & $(CXX) -g -c $(CFLAGS) $(INC) $< -o $@

obj/%_debug.o: src/%.cpp
	@echo CXX  $< & $(CXX) -g -c $(CFLAGS) $(INC) $< -o $@

obj/%_debug.o: src/Windows/%.cpp
	@echo CXX  $< & $(CXX) -g -c $(CFLAGS) $(INC) $< -o $@

obj/%.res: src/Windows/%.rc
	@echo RES  $< & windres $(INC) -J rc -O coff -i $< -o $@

$(TGT): $(OBJECTS) $(RES)
	@echo LINK $@ & $(CXX) -shared -Wl,--dll $(OBJECTS) -o $@ -s $(LFLAGS)

$(TGT_D): $(OBJECTS_D) $(RES)
	@echo LINK $@ & $(CXX) -shared -Wl,--dll $(OBJECTS_D) -o $@ $(LFLAGS)

makedirs:
	@if not exist obj mkdir obj
	@if not exist bin mkdir bin

clean:
	@if exist obj rd /s /q obj
	@if exist bin rd /s /q bin

-include ${DEPENDS}
-include ${DEPENDS_D}
