# Autores: 
#	Darío Adrián Hernández Barroso
#	Ángel Manuel Martín
#

CC = gcc 
#CFLAGS = -Wall -g -Werror
CFLAGS = -Wall 
LDFLAGS = 

EXE = ejercicio2 ejercicio3 ejercicio5 ejercicio5b

all: $(EXE)
 
.PHONY: clean
clean:
	rm -f *.o core $(EXE)

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

ejercicio5 : ejercicio5.o semaforos.o
	@echo "#---------------------------"
	@echo "# Generando $@ "
	@echo "# Depende de $^"
	@echo "# Ha cambiado $<"
	$(CC) $(CFLAGS) -o $@ semaforos.o $@.o $(LDFLAGS)

ejercicio5b : ejercicio5b.o semaforos.o
	@echo "#---------------------------"
	@echo "# Generando $@ "
	@echo "# Depende de $^"
	@echo "# Ha cambiado $<"
	$(CC) $(CFLAGS) -o $@ semaforos.o $@.o $(LDFLAGS)

