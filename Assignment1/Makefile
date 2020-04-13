CC = g++ -std=c++11
CFLAGS = -O3 -fopenmp
INCFLAGS = -I"./FreeImage/" -I"./Source/"
LDFLAGS = -L"./FreeImage/" -lfreeimage

all : RenderScene

RenderScene : ./Source/*.cpp
	${CC} ${CFLAGS} ${INCFLAGS} ./Source/*.cpp ${LDFLAGS} -o RenderScene

clean:
	  rm -rf RenderScene
