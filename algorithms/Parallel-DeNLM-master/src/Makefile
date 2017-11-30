#
# 'make depend' uses makedepend to automatically generate dependencies 
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = g++ -g
# define any compile-time flags
CFLAGS = -fopenmp
LIBS = `pkg-config --libs opencv` -lopencv_highgui 

LDFLAGS = -g3 -Wall 
# define any directories containing header files other than /usr/include
#
INCLUDES= -I../include -I/usr/local/include/opencv -I/usr/local/include/opencv2




# define the C source files
SRCS = bfilterDeceived.cpp nlmfilterDeceived.cpp  noAdaptiveLaplacian.cpp  parallelDeWAFF.cpp  tools.cpp

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = mycc

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:
	$(CC) $(OBJS) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(LDFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.c.o: $(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean: rm *.o *~ $(MAIN)

depend: $(SRCS) makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it













