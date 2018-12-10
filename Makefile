SRCDIR =src
INCDIR =include
BUILDDIR =build
BINDIR =bin

CC = g++
CCFLAGS = -g -Wall -std=c++17
INC = -I$(INCDIR)
TARGET = $(BINDIR)/ipmt

SRCEXT=cpp
SOURCES = $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

$(TARGET): $(OBJECTS)
	@echo "Vinculando..."
	@mkdir -p $(BINDIR)
	$(CC) -o $@ $^ $(CCFLAGS) $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT) compilando
	@mkdir -p $(BUILDDIR)
	$(CC) -c -o $@ $< $(CCFLAGS) $(INC)

.PHONY: clean compilando

clean:
	@echo "Limpando..."
	$(RM) -r $(BUILDDIR)/ $(BINDIR)/

compilando:
	@echo "Compilando..."

debug:
	$(CC) -o debug 
