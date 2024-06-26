SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic
CPLEXDIR   = /opt/ibm/ILOG/CPLEX_Studio2211/cplex
CONCERTDIR = /opt/ibm/ILOG/CPLEX_Studio2211/concert

################

# Compilador 
CCC = g++ -O0

# Opcoes de compilacao
CCOPT = -m64 -O -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD -Wno-ignored-attributes

# Bibliotecas e includes
CPLEXBINDIR   = $(CPLEXDIR)/bin/$(SYSTEM)
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

CCLNDIRS  = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR)
CCLNFLAGS = -lconcert -lilocplex -lcplex -lm -lpthread -ldl

CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include

CCFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) 

all : Trabalho_1

Trabalho_1: Trabalho_1.o
	$(CCC) $(CCFLAGS) -o Trabalho_1 Trabalho_1.o $(CCLNDIRS) $(CCLNFLAGS)
Trabalho_1.o: Trabalho_1.cpp
	$(CCC) -c $(CCFLAGS) Trabalho_1.cpp -o Trabalho_1.o

clean:
	rm --force Trabalho_1 Trabalho_1.o

