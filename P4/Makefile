# Autores:
#	Darío Adrián Hernández Barroso
#	Ángel Manuel Martín
#

CC = gcc
CFLAGS = -Wall -g -Werror
#CFLAGS = -Wall
LDFLAGS =

EXE = cadena_montaje

all: $(EXE) doc

.PHONY: clean
clean: clean_doc
	rm -f *.o core $(EXE)

clean_doc:
	rm -rf doc

doc: clean_doc
	doxygen Doxyfile

$(EXE): % : %.o
	@echo "#---------------------------"
	@echo "# Generando $@ "
	@echo "# Depende de $^"
	@echo "# Ha cambiado $<"
	$(CC) $(CFLAGS) -o $@ $@.o $(LDFLAGS)

%.o : %.c
	@echo "#---------------------------"
	@echo "# Generando $@"
	@echo "# Depende de $^"
	@echo "# Ha cambiado $<"
	$(CC) $(CFLAGS) -c $<
