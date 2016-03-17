# Autores: 
#	Darío Adrián Hernández Barroso
#	Ángel Manuel Martín
#

CC = gcc 
#CFLAGS = -Wall -g
CFLAGS =  -Wall -Werror
LDFLAGS = -pthread

EXE = ejercicio2 ejercicio3a ejercicio3b ejercicio4 ejercicio4proc ejercicio6 ejercicio8 ejercicio10

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

