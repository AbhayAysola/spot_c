DEPS := libcurl json-c

all: build/main

build/main: build/main.o build/ap.o
	gcc $? $$(pkg-config --libs --cflags $(DEPS)) -o build/main

build/main.o : src/main.c
	gcc $? -I include -o $@ -c

build/ap.o : src/ap.c
	gcc $? -I include -o $@ -c

.PHONY: clean
clean:
	@rm build/* -f
	@echo "Cleaned"