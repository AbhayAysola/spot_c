DEPS := libcurl json-c 'libprotobuf-c >= 1.0.0' openssl

all: build/main

.PHONY: run
run: clean proto build/main
	clear
	@build/main

build/main: build/main.o build/ap.o build/diffie_hellman.o build/connection.o
	gcc $? protocol/*.pb-c.c -I include $$(pkg-config --libs --cflags $(DEPS)) -o build/main -g

build/main.o : src/main.c
	gcc $? -I include -o $@ -c -g

build/ap.o : src/ap.c
	gcc $? -I include -o $@ -c -g

build/diffie_hellman.o: src/diffie_hellman.c
	gcc $? -I include -o $@ -c -g

build/connection.o: src/connection.c
	gcc $? -I include -o $@ -c -g

debug: build/main
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=debug ./build/main 


.PHONY: proto
proto:
	@protoc --c_out=. protocol/*
	@mv protocol/*pb-c.h include/
	@sed -i -e 's/protocol\///' protocol/keyexchange.pb-c.c
	@echo "Proto files generated"

.PHONY: clean
clean:
	@rm build/* -f
	@rm protocol/*pb-c.c -f
	@rm include/*pb-c.h -f
	@echo "Cleaned"

.PHONY: format
format:
	@astyle --options="astyle-code-format.cfg" "src/*.c,*.h" "include/*.c,*.h"