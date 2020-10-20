##
# Main makefile to build the controller application
#

CC := g++
CCVERSION = $(shell $(CC) -dumpversion | awk -F'.' '{print $$1}')
SRCDIR := src
BUILDDIR := build
RM := rm
BINDIR := bin
TARGET := $(BINDIR)/controller

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g

# Add support for C++2a
ifeq ($(shell test $(CCVERSION) -le 10; echo $$?), 0)
	CFLAGS += -std=c++2a
endif

# mosquitto - Mosquitto MQTT Broker Library (sudo apt-get install -y libmosquitto-dev)
# jsoncpp - JSON parsing Library (sudo apt-get install -y libjsoncpp-dev)
# pqxx - C++ postgreSQL library, depends on pq so it's listed first (sudo apt-get install -y libpqxx-dev)
# pq - C postgreSQL library (sudo apt-get install -y libpq-dev)
LIB := -L lib -lmosquitto -lpqxx -lpq -ljsoncpp -pthread
INC := -I include


all: $(TARGET)

# Build application
$(TARGET): $(OBJECTS)
	@echo "==> Linking application"
	@mkdir -p $(BINDIR)
	@$(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo "==> Compiling source $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo "==> Cleaning artifacts"
	@$(RM) -rf $(BUILDDIR) $(TARGET)

tarball:
	@echo "==> Building controller package tarball"
	@tar -cz -f docker/controller/controller.tgz include Makefile src

image: tarball
	@echo "==> Build image"
	@cd ./docker/controller && docker build . -t localhost:5000/buffaloiot/controller:latest

install:
	@echo "==> Installing controller"
	@cp ./bin/controller /usr/bin/controller

.PHONY: tarball image clean install
