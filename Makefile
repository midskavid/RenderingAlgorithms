CC = g++ -std=c++11
INCFLAGS = -I"./FreeImage/" -I"./Source/"
LDFLAGS = -L"./FreeImage/" -lfreeimage

all : RenderScene

RenderScene : ./Source/*.cpp
	${CC} ${CFLAGS} ${INCFLAGS} ./Source/*.cpp ${LDFLAGS} -o RenderScene

clean:
	  rm -rf RenderScene
