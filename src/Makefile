PATHI = \
	-Iinclude \
	-I/opt/local/include/SDL \
	-Iexternal 

LIB = \
	-L/usr/local/lib \
	-L/usr/X11R6/lib \
	-L"/System/Library/Frameworks/OpenGL.framework/Libraries" \

FRAMEWORK = -framework OpenGL -framework Cocoa


LIBRARIES = \
	-lX11 \
	-lXi \
	-lXmu \
	-lGL \
	-lGLU \
	-lSDL \
	-lSDLmain \
	-lpthread \
	-lSDL_image \
	-lSDL_ttf \
	-lstdc++

CC = /usr/bin/g++
CPPFLAGS = -D_REENTRANT -Wno-deprecated $(PATHI)

ALL_OBJS = \
	RPiece.o \
	RCamera.o \
	RUtils.o \
	Math3D.o \
	REngine.o \
	RInputSystem.o \
	RWorld.o \
	md2.o \
	RResources.o \
	RPlayer.o \
	RTrack.o \
	RPhysics.o \
	RBSPtree.o \
	REnclosingDome.o \
	RScenery.o \
	RGameLogic.o \
	RFontManager.o \
	gmMat3.o \
	gmMat4.o \
	RObjectBase.o \
	RGameMenu.o \
	ROnScreenDisplay.o \
	RBillBoard.o \
	RParticle.o

OBJS = main.o $(ALL_OBJS)
TEST_OBJS = RTestMain.o test.o $(ALL_OBJS)

MYTEST_OBJS = mytest.o 

all: $(OBJS)
	$(CC) $(FRAMEWORK) $^ -o racer $(LIB) $(LIBRARIES)

test: $(TEST_OBJS)
	$(CC) $^ -o test_racer $(LIB) $(LIBRARIES)

mytest: $(MYTEST_OBJS)
	$(CC) $(LIB) $^ -o test_racer $(LIBRARIES)

clean:
	rm -f *.o racer test_racer
