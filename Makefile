# Makefile for building NppFTP

ifeq ($(OS),Windows_NT)
WINDRES = windres
RMDIR   = if exist $(1) rd /s /q $(1)
else
WINDRES = i686-w64-mingw32-windres
RMDIR   = rm -fr $(1)
endif

CXX    = i686-w64-mingw32-g++
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
debug:   bin obj $(TGT_D)
release: bin obj $(TGT)
	@echo ============ creating NppFTP.zip ============
	@zip -9 -r NppFTP.zip $(TGT) doc/
test:    bin obj $(TGT)
	@copy /y $(TGT) "%APPDATA%\Notepad++\plugins" >nul
	@cmd /c start notepad++

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
	@echo RES  $< & $(WINDRES) $(INC) -J rc -O coff -i $< -o $@

$(TGT): $(OBJECTS) $(RES)
	@echo LINK $@ & $(CXX) -shared -Wl,--dll $(OBJECTS) $(RES) -o $@ -s $(LFLAGS)

$(TGT_D): $(OBJECTS_D) $(RES)
	@echo LINK $@ & $(CXX) -shared -Wl,--dll $(OBJECTS_D) $(RES) -o $@ $(LFLAGS)

bin:
	@mkdir bin

obj:
	@mkdir obj

clean:
	@$(call RMDIR, bin)
	@$(call RMDIR, obj)

-include ${DEPENDS}
-include ${DEPENDS_D}
