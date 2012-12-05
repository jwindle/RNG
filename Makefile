
# Include for Matrix and GSL.
UINC = -I$(HOME)/Code/Matrix/ \
       -I$(HOME)/Code/RNG/ \
       -I$(HOME)/Code/include/

GLIB = -L$(HOME)/Code/lib

RINC = $(shell R CMD config --cppflags)
RLNK = $(shell R CMD config --ldflags)

# USE make target USE_R=-DUSE_R to use R.
# USE_R =

OPT = -O2 $(USE_R) -pedantic -ansi -Wshadow -Wall

ifdef USE_R
INC = $(UINC) $(RINC)
LNK = $(RLNK)
DEP = RRNG.o
else
INC = $(UINC)
LNK = $(GLIB) -lgsl
DEP = GRNG.o
endif

gtest : test.c RNG.o 
	g++ test.c $(DEP) $(INC) $(OPT) libgrng.so -o test $(LNK) -lblas -llapack

rtest : librrng.so
	g++ test.c $(INC) $(OPT) librrng.so -o test -lblas -llapack

glibtest :
	g++ $(INC) $(GLIB) libtest.cpp -fPIC -shared -o libtest.so -lgsl -lblas -llapack

rlibtest :
	g++ $(INC) $(RINC) -DUSE_R libtest.cpp -fPIC -shared -o libtest.so -lblas -llapack $(RLNK)

librrng.so : RNG.o RRNG.o
	g++ $(OPT) -DUSE_R RNG.o RRNG.o -fPIC -shared -o librrng.so $(RLNK)

libgrng.so : RNG.o GRNG.o
	g++ $(OPT) RNG.o GRNG.o -fPIC -shared -o libgrng.so $(LNK)

# You can use the static flag to force compiling with static libraries.
librrng.a : RNG.o RRNG.o
	ar -cvq librrng.a RNG.o RRNG.o

libgrng.a : RNG.o GRNG.o
	ar -cvq libgrng.a RNG.o GRNG.o

RNG.o : RNG.hpp RNG.cpp $(DEP)
	g++ $(INC) $(OPT) -c RNG.cpp -o RNG.o 

GRNG.o: GRNG.cpp GRNG.hpp
	g++ $(INC) $(OPT) -c GRNG.cpp -o GRNG.o

RRNG.o: RRNG.cpp RRNG.hpp
	g++ $(INC) $(OPT) -DUSE_R -c RRNG.cpp -o RRNG.o

GRNG :
	g++ $(INC) $(GLIB) RNG.h -fPIC -shared -o librng.so -lgsl -lblas -llapack

RRNG :
	g++ $(INC) $(RINC) -DUSE_R RNG.h -fPIC -shared -o librng.so -lblas -llapack $(RLNK)

rm.o :
	rm *.o

