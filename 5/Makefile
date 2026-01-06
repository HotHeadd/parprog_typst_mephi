OMPI_CXX=/opt/homebrew/opt/llvm/bin/clang++
CLANG = /opt/homebrew/opt/open-mpi/bin/mpic++
CLANG_FLAGS = -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -std=c++20

OUTPUT_DIR = bin
TARGET = codes/main.cpp
BINS = $(patsubst codes/%.cpp,$(OUTPUT_DIR)/%,$(TARGET))

RESULTS = $(wildcard results/*)
GRAPHS = $(patsubst results/%,graphs/%.png,$(RESULTS))

NP = 8

all: clean run graphs

$(OUTPUT_DIR)/%: codes/%.cpp
	@mkdir -p $(OUTPUT_DIR)
	$(CLANG) $< $(CLANG_FLAGS) -o $@

# Запуск программы: запускаем для np=1..$(NP)
run: $(BINS)
	@echo "=== Запуск бинарников (np = 1..$(NP)) ==="
	for bin in $(BINS); do \
		for np in `seq 1 $(NP)`; do \
			echo "Запуск $$bin с $$np процессами..."; \
			mpirun -np $$np ./$$bin; \
		done; \
	done
	@echo "=== Данные сохранены в results/ ==="

# Построение графиков (можно вызывать отдельно)
graphs:
	@mkdir -p graphs
	@echo "=== Построение графиков ==="
	for f in $(wildcard results/*); do \
		name=$$(basename $$f); \
		echo "Построение графика для $$name"; \
		python3 ../graphs_tool.py $$f -o graphs/$$name.png --dpi 800; \
	done
	@echo "=== Графики обновлены ==="

clean:
	rm -rf $(OUTPUT_DIR)

.PHONY: all run graphs clean init render

init:
	mkdir -p bin codes graphs results preamble

render:
	typst compile --root .. report.typ
