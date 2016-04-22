# Autores:
#	Darío Adrián Hernández Barroso
#	Ángel Manuel Martín
#

CC = gcc
CFLAGS = -Wall -g
#CFLAGS = -Wall
LDFLAGS = -pthread

EXE = cadena_montaje
SEM_EXE = final

all: $(EXE) $(SEM_EXE)

.PHONY: clean
clean: clean_doc
	rm -f *.o core $(EXE) $(SEM_EXE) semaforos.o

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

