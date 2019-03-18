SRC_DIR := main
INCLUDE_DIR := include
O_DIR := build
OUT_DIR := bin


LD := g++
LDFLAGS := -std=c++11 -I./$(INCLUDE_DIR)

$(OUT_DIR)/DELTA: $(patsubst %cpp, %o, $(wildcard $(SRC_DIR)/*.cpp))
	@mkdir -p $(OUT_DIR)
	$(LD) $(LDFLAGS) $^ -o $@
	@mkdir -p $(O_DIR)
	@mv $(SRC_DIR)/*.o $(O_DIR)/

%.o: %.cpp
	$(LD) $(LDFLAGS) -c $< -o $@

clean:
	@rm -rf ./$(O_DIR)/*
	@rm -rf ./$(OUT_DIR)/*