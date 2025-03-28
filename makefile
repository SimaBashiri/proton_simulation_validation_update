# Compiler settings
CXX := g++
CXXFLAGS := -std=c++11 -Wall -Werror -g
ROOT_FLAGS := $(shell root-config --cflags --libs)

# Targets
.PHONY: all clean  # Crucial: Declare these as phony targets

all: collect_systematics
	@echo "Build completed successfully"

collect_systematics: collect_systematics.cc
	@echo "Building $@..."
	$(CXX) $(CXXFLAGS) -o $@ $< $(ROOT_FLAGS)
	@touch $@  # Explicitly update timestamp

clean:
	rm -f collect_systematics