CC=g++
CPPFLAGS=-MD -MP
CFLAGS=-c -Wall -fpermissive -Wno-write-strings
LDFLAGS=
SOURCES=circuit.cpp circuitcheck.cpp circuitoptimise.cpp circuitoutput.cpp compile.cpp compilestate.cpp estimate.cpp expand.cpp hcast.cpp lex.yy.c hc.tab.c main.cpp simoutput.cpp symtab.cpp typecheck.cpp types.cpp vhdloutput.cpp widenum.cpp
OBJECTS_TMP=$(SOURCES:.cpp=.o)
OBJECTS=$(OBJECTS_TMP:.c=.o)
EXECUTABLE=smallhc

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

hc.tab.c : hc.y
	bison -d -v $<

lex.yy.c : hc.l
	flex -d -o lex.yy.c $<

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@
	
.cpp.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@
	
-include $(OBJECTS:.o=.d)
	