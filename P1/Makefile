# Autores: 
#	Darío Adrián Hernández Barroso
#	Ángel Manuel Martín
#

CC = gcc 
#CFLAGS = -Wall -g
CFLAGS =  -Wall -Werror

EXE = ejercicio4 ejercicio5 ejercicio6 ejercicio8 ejercicio9 cat

all: $(EXE)
 
.PHONY: clean
clean:
	rm -f *.o core $(EXE)

$(EXE): % : %.o
	@echo "#---------------------------"
	@echo "# Generando $@ "
	@echo "# Depende de $^"
	@echo "# Ha cambiado $<"
	$(CC) $(CFLAGS) -o $@ $@.o  

%.o : %.c
	@echo "#---------------------------"
	@echo "# Generando $@"
	@echo "# Depende de $^"
	@echo "# Ha cambiado $<"
	$(CC) $(CFLAGS) -c $<

