JOBS ?= 5

ifeq ($(OS),Windows_NT)
ECHO  = @echo
RM    = del /F /Q
RMDIR = rmdir /S /Q
else
ECHO  = @echo
RM    = rm -f
RMDIR = rm -rf
endif

all: build
	cmake --build build/ -j $(JOBS)

build:
	cmake -G "Unix Makefiles" -S . -B build/

.PHONY: clean
clean:
	cmake --build build/ --target clean

.PHONY: veryclean
veryclean:
	$(RMDIR) build coverage.html

help:
	$(ECHO) "Usage: make [target]"
	$(ECHO) ""
	$(ECHO) "Targets:"
	$(ECHO) " all        default - Generate the build/ tree, build and run all tests"
	$(ECHO) " clean      perform a clean build, do not remove the build/ tree"
	$(ECHO) " veryclean  remove the build/ tree and start fresh"
