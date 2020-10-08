##
# Main makefile to build the controller application
#

CC := g++
SRCDIR := src
BUILDDIR := build
RM := rm
BINDIR := bin
TARGET := $(BINDIR)/controller

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g
# mosquitto - Mosquitto MQTT Broker Library (sudo apt-get install -y libmosquitto-dev)
# pqxx - C++ postgreSQL library, depends on pq so it's listed first (sudo apt-get install -y libpqxx-dev)
# pq - C postgreSQL library (sudo apt-get install -y libpq-dev)
LIB := -L lib -lpthread -lmosquitto -lpqxx -lpq
INC := -I include

all: $(TARGET)

# Build application
$(TARGET): $(OBJECTS)
	@echo "==> Linking application"
	@mkdir -p $(BINDIR)
	@$(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo "==> Compiling source"
	@mkdir -p $(BUILDDIR)
	@$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo "==> Cleaning artifacts"
	@$(RM) -r $(BUILDDIR) $(TARGET)

.PHONY: clean
