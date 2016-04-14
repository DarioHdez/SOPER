# Autores:
#	Darío Adrián Hernández Barroso
#	Ángel Manuel Martín
#

CC = gcc
#CFLAGS = -Wall -g -Werror
CFLAGS = -Wall
LDFLAGS =

EXE = ejercicio2 ejercicio3
SEM_EXE = ejercicio5 ejercicio5b

all: $(EXE) $(SEM_EXE)

.PHONY: clean
clean:
	rm -f *.o core $(EXE) $(SEM_EXE)

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
