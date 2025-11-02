CLANG = /opt/homebrew/opt/llvm/bin/clang
CLANG_FLAGS = -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -fopenmp

OUTPUT_DIR = bin
# TARGET = codes/sequential.c codes/parallel.c
TARGET = codes/parallel.c
BINS = $(patsubst codes/%.c,$(OUTPUT_DIR)/%,$(TARGET))

all: $(BINS) run_graphs

$(OUTPUT_DIR)/%: codes/%.c
	@mkdir -p $(OUTPUT_DIR)
	$(CLANG) $< $(CLANG_FLAGS) -o $@

run_graphs: $(BINS)
	@echo "=== Запуск всех бинарников ==="
	for bin in $(BINS); do \
		echo "Запуск $$bin..."; \
		./$$bin; \
	done
	@echo "=== Построение графиков ==="
	for f in $(wildcard results/*); do \
		name=$$(basename $$f); \
		echo "Построение графика для $$name"; \
		python3 ../graphs_tool.py $$f -o graphs/$$name.png --dpi 800; \
	done

clean:
	rm -rf $(OUTPUT_DIR)

.PHONY: init

init:
	mkdir -p bin
	mkdir -p codes
	mkdir -p graphs
	mkdir -p results
	mkdir -p preamble
