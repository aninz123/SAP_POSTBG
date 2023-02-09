###########
#
# Makefile.
# Accenture - AIS
#
###########
###########
#RBM 9.0.4 Upgrade
#Srikanth Reddy Kosanam : 
#Changed the LINKFLAG for Linux platform
#Changed the paths to relative paths
########################################

ifeq ($(NC_BUILD),1)
	RBM_VER=-D'RBM_VERSION="9.0.4"'
	SVN_VER=-D'SVN_REV="$(shell svnversion -n .)"'
	NEW_CC=$(CC) $(RBM_VER) $(SVN_VER)
else
	NEW_CC=$(CC)
endif

INCDIR=../../include/SAP_POSTBG
GLOBALINC=../../../global/include

FILES=bl_sap_postbg.cpp $(GLOBALINC)/bl_utils.cpp
OBJECTS=bl_sap_postbg.o bl_utils.o


OUTFILE=BLC0080501_SAP_POSTBG

VPATH=./objects:./bin

all : transform

LINKFLAG =-L$(ORACLE_HOME)/lib \
	-L/apps/oracle/product/default/lib

COMPFLAG = -I$(ORACLE_HOME)/rdbms/demo \
	-I$(ORACLE_HOME)/plsql/public \
	-I$(ORACLE_HOME)/network/public \
	-I$(ORACLE_HOME)/rdbms/public  \
	-I$(INCDIR) \
	-I$(GLOBALINC)

transform: $(OBJECTS)
	$(CC) -o $(OUTFILE) $(OBJECTS) $(COMPFLAG) $(LINKFLAG) -lc -ldl -lm -lclntsh -xarch=v9
	rm $(OBJECTS)
	#cp $(OUTFILE) $(INFINYS_ROOT)/bin/

$(OBJECTS): $(FILES)
	$(NEW_CC) $(RBM_VER) $(SVN_VER) ${COMPFLAG} -g -c $(FILES) -m64 -errchk=longptr64
	
clean:
	/bin/rm -f core *.o $(OUTFILE)
