
# Include for Matrix and GSL.
INC = -I$(HOME)/Code/Matrix/ \
	-I$(HOME)/Code/RNG/ \
	-I$(HOME)/Code/include/

GLIB = -L$(HOME)/Code/lib

RINC = $(shell R CMD config --cppflags)
RLNK = $(shell R CMD config --ldflags)

gtest :
	g++ test.c $(INC) $(GLIB) -lgsl -lblas -llapack -o test

rtest :
	g++ test.c $(INC) $(RINC) -DUSE_R $(RLNK) -lblas -llapack -o test

glibtest :

	g++ $(INC) $(GLIB) libtest.cpp -fPIC -shared -o libtest.so -lgsl -lblas -llapack

rlibtest :
	g++ $(INC) $(RINC) -DUSE_R libtest.cpp -fPIC -shared -o libtest.so -lblas -llapack $(RLNK)

GRNG :
	g++ $(INC) $(GLIB) RNG.h -fPIC -shared -o librng.so -lgsl -lblas -llapack

RRNG :
	g++ $(INC) $(RINC) -DUSE_R RNG.h -fPIC -shared -o librng.so -lblas -llapack $(RLNK)

clean :
	rm *.o

