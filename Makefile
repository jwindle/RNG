
# Include for Matrix and GSL.
UINC = -I$(HOME)/Code/Matrix/ \
       -I$(HOME)/Code/RNG/ \
       -I$(HOME)/Code/include/

GLIB = -L$(HOME)/Code/lib

RINC = $(shell R CMD config --cppflags)
RLNK = $(shell R CMD config --ldflags)

# USE make target USE=R to use R.
# USE make target USE=GRNGPar to use GRNG

# Manual override
# USE_R =

ifdef USE
	ifeq ($(USE), R)
		INC = $(UINC) $(RINC)
		LNK = $(RLNK)
		DEP = RRNG.o
		USE_R = -DUSE_R
	else 
		INC = $(UINC)
		LNK = $(GLIB) -lgsl
		DEP = GRNGPar.o
	endif
else
	INC = $(UINC)
	LNK = $(GLIB) -lgsl
	DEP = GRNG.o
endif

OPT = -O2 $(USE_R) -pedantic -ansi -Wshadow -Wall
OPT = $(USE_R) -pedantic -ansi -Wshadow -Wall

test_parallel : test_parallel.cpp RNGParallel.hpp CPURNG.hpp RNG.o 
	g++ test_parallel.cpp $(DEP) $(INC) $(OPT)  libgrng.so -o test_parallel $(LNK) -fopenmp -lblas -llapack

gpartest : test.c RNG.o
	g++ test.c $(DEP) $(INC) $(OPT) libgrngpar.so -o test $(LNK) -lblas -llapack

gtest : test.c libgrng.so 
	g++ test.c $(DEP) $(INC) $(OPT) libgrng.so -o test $(LNK) -lblas -llapack

rtest : librrng.so
	g++ test.c $(INC) $(OPT) librrng.so -o test -lblas -llapack

glibtest :
	g++ $(INC) $(GLIB) libtest.cpp -fPIC -shared -o libtest.so -lgsl -lblas -llapack

rlibtest :
	g++ $(INC) $(RINC) -DUSE_R libtest.cpp -fPIC -shared -o libtest.so -lblas -llapack $(RLNK)

libgrngpar.so : RNG.o GRNGPar.o
	g++ $(OPT) -DUSE_GRNGPAR RNG.o GRNGPar.o -fPIC -shared -o libgrngpar.so $(LNK)

librrng.so : RNG.o RRNG.o
	g++ $(OPT) -DUSE_R RNG.o RRNG.o -fPIC -shared -o librrng.so $(RLNK)

libgrng.so : RNG.o GRNG.o
	g++ $(OPT) RNG.o GRNG.o -fPIC -shared -o libgrng.so $(LNK)

# You can use the static flag to force compiling with static libraries.
librrng.a : RNG.o RRNG.o
	ar -cvq librrng.a RNG.o RRNG.o

libgrng.a : RNG.o GRNG.o
	ar -cvq libgrng.a RNG.o GRNG.o

RNGPar.o : RNGPar.cpp RNGPar.hpp
	g++ $(INC) $(OPT) -c RNGPar.cpp -o RNGPar.o

GRNGPar.o : GRNGPar.cpp GRNGPar.hpp
	g++ $(INC) $(OPT) -c GRNGPar.cpp -o GRNGPar.o

RNG.o : RNG.hpp RNG.cpp $(DEP)
	g++ $(INC) $(OPT) -c RNG.cpp -o RNG.o -fPIC

GRNG.o: GRNG.cpp GRNG.hpp
	g++ $(INC) $(OPT) -c GRNG.cpp -o GRNG.o -fPIC

RRNG.o: RRNG.cpp RRNG.hpp
	g++ $(INC) $(OPT) -DUSE_R -c RRNG.cpp -o RRNG.o -fPIC

GRNG :
	g++ $(INC) $(GLIB) RNG.h -fPIC -shared -o librng.so -lgsl -lblas -llapack

RRNG :
	g++ $(INC) $(RINC) -DUSE_R RNG.h -fPIC -shared -o librng.so -lblas -llapack $(RLNK)

print : 
	echo $(INC)
	echo $(OPT)

rm.o :
	rm *.o

clean :
	rm *.o
