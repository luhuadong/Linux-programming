SCRIPT=install.sh
BIN=hello
RUN=helloinstall.run

all:
	gcc hello.c -o $(BIN)
	tar jcvf $(BIN).tar.bz2 $(BIN)
	cat $(SCRIPT) $(BIN).tar.bz2 > $(RUN)
	chmod +x $(RUN)

clean:
	rm $(BIN) $(BIN).tar.bz2
	rm $(RUN)
