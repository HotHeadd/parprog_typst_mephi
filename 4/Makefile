CLANG = /opt/homebrew/opt/llvm/bin/clang++
CLANG_FLAGS = -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -fopenmp -std=c++20

OUTPUT_DIR = bin
TARGET = codes/out_pars.cpp
BINS = $(patsubst codes/%.cpp,$(OUTPUT_DIR)/%,$(TARGET))

RESULTS = $(wildcard results/*)
GRAPHS = $(patsubst results/%,graphs/%.png,$(RESULTS))

# Основная цель: всё сразу
all: run graphs

# Компиляция
$(OUTPUT_DIR)/%: codes/%.cpp
	@mkdir -p $(OUTPUT_DIR)
	$(CLANG) $< $(CLANG_FLAGS) -o $@

# Запуск программы
run: $(BINS)
	@echo "=== Запуск бинарников ==="
	for bin in $(BINS); do \
		echo "Запуск $$bin..."; \
		./$$bin; \
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
