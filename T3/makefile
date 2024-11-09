# Nombre del archivo principal (sin la extensión .java)
MAIN_CLASS = Main

# Encuentra todos los archivos .java en el directorio actual
SOURCES = $(wildcard *.java)

# Objetivo por defecto: compilar y ejecutar el proyecto
all: build run

# Compilar todos los archivos .java
build:
	@echo "Compilando archivos .java..."
	javac $(SOURCES)

# Ejecutar la clase principal
run:
	@echo "Ejecutando $(MAIN_CLASS)..."
	java $(MAIN_CLASS)

# Limpiar los archivos .class generados
clean:
	@echo "Limpiando archivos compilados..."
	rm -f *.class

# Ayuda para mostrar cómo usar el Makefile
help:
	@echo "Makefile para compilar y ejecutar un proyecto Java"
	@echo "Uso:"
	@echo "  make all      - Compilar y ejecutar el proyecto"
	@echo "  make build    - Solo compilar el proyecto"
	@echo "  make run      - Ejecutar el proyecto (después de compilar)"
	@echo "  make clean    - Limpiar archivos compilados"
