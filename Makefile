# For OS X

CC=clang++
# I got tired of seeing all the warnings that came with the program during a build, so I disabled them
# CFLAGS=-Wall -std=c++11 -g -DDEBUG
CFLAGS=-Wno-everything -std=c++11 -g -DDEBUG

SRC=./src
OUT=./build
FLTK=./fltk
GLEW=./glew
X11=/opt/X11
FILEIO=./src/fileio
PARSER=./src/parser
SCENE=./src/scene
SCENEOBJECTS=./src/SceneObjects
UI=./src/ui
VECMATH=./src/vecmath

LIBDIRS=-L$(FLTK)/lib -L$(GLEW)/lib -L$(X11)/lib
LIBS=-lfltk -lfltk_forms -lfltk_gl -lfltk_images -lGLEW -lX11
INCLUDES=-I$(FLTK)/include -I$(GLEW)/include -I$(X11)/include
FRAMEWORKS=-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

target = $(OUT)/RayTracer
sources = $(wildcard $(SRC)/*.cpp $(SRC)/*.c $(SRC)/*.C $(FILEIO)/*.cpp $(FILEIO)/*.c $(FILEIO)/*.C $(PARSER)/*.cpp $(PARSER)/*.c $(PARSER)/*.C $(SCENEOBJECTS)/*.cpp $(SCENEOBJECTS)/*.c $(SCENEOBJECTS)/*.C $(UI)/*.cpp $(UI)/*.c $(UI)/*.C $(UI)/*.cxx $(VECMATH)/*.cpp $(VECMATH)/*.c $(VECMATH)/*.C $(SCENE)/*.cpp $(SCENE)/*.c $(SCENE)/*.C)

$(target):	$(sources) $(wildcard $(SRC)/*.hpp $(SRC)/*.h $(SRC)/*.H $(FILEIO)/*.hpp $(FILEIO)/*.h $(FILEIO)/*.H $(PARSER)/*.hpp $(PARSER)/*.h $(PARSER)/*.H $(SCENEOBJECTS)/*.hpp $(SCENEOBJECTS)/*.h $(SCENEOBJECTS)/*.H $(UI)/*.hpp $(UI)/*.h $(UI)/*.H $(VECMATH)/*.hpp $(VECMATH)/*.h $(VECMATH)/*.H $(SCENE)/*.hpp $(SCENE)/*.h $(SCENE)/*.H)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBDIRS) $(LIBS) $(FRAMEWORKS) $(sources) -o $@

clean:
	rm -f $(target)
