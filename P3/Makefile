# Autores:
#	Darío Adrián Hernández Barroso
#	Ángel Manuel Martín
#

CC = gcc
#CFLAGS = -Wall -g -Werror
CFLAGS = -Wall
LDFLAGS = -pthread

EXE = ejercicio2 ejercicio3
SEM_EXE = ejercicio5 ejercicio5b ejercicio6 ejercicio7

all: $(EXE) $(SEM_EXE) doc

.PHONY: clean
clean: clean_doc
	rm -f *.o core $(EXE) $(SEM_EXE)

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

$(SEM_EXE): % : %.o semaforos.o
	@echo "#---------------------------"
	@echo "# Generando $@ "
	@echo "# Depende de $^"
	@echo "# Ha cambiado $<"
	$(CC) $(CFLAGS) -o $@ semaforos.o $@.o $(LDFLAGS)
