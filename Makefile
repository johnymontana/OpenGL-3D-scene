uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
#CFLAGS += -I.
#Linux (default)
#EXE = hw3
LDFLAGS = -lGL -lGLU -lglut
	
#OS X
ifeq ($(uname_S), Darwin)
	LDFLAGS = -framework Carbon -framework OpenGL -framework GLUT
endif

#$(EXE) : geom.c fontutil.c precision.c mvm.c manip_teapot.c
#	 gcc -Wall -o $@ $< $(CFLAGS) $(LDFLAGS)

proj2: list.cpp obj_parser.cpp objLoader.cpp string_extra.cpp GLee.c gltools.cpp math3d.cpp proj2.cpp
	g++ -o proj2 list.cpp obj_parser.cpp objLoader.cpp string_extra.cpp GLee.c gltools.cpp math3d.cpp proj2.cpp -I. $(LDFLAGS)
