LIBS :=
CFLAGS := -Wall -Wconversion -std=c++11
LDFLAGS :=

CFLAGS += -g --coverage
LDFLAGS += -g --coverage

INC_DIRS := include/json source
OBJ_DIR := obj
BIN_DIR := bin

TEST_SOURCES := $(shell find tests/ -name "*.cpp")

TEST_OBJECTS := $(patsubst %, $(OBJ_DIR)/%.o, $(TEST_SOURCES))

CLEAN_FILES := $(OBJ_DIR) $(BIN_DIR)
DEPS := $(TEST_OBJECTS:.o=.d)

all: test

bin/test: $(TEST_OBJECTS)
	@mkdir -p $(@D)
	g++ $(LDFLAGS) $(filter %.o,$^) -Lbin $(addprefix -l, $(LIBS)) -o $@
$(OBJ_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	g++ $(CFLAGS) $(addprefix -I, $(INC_DIRS)) -c  -MMD -MP $< -o $@

test: bin/test
	@mkdir -p coverage
	rm -f coverage/all.info coverage/coverage.info
	rm -f $(shell find $(OBJ_DIR)/ -name "*.gcda")
	bin/test
	lcov --capture --directory obj --base-directory . --output-file coverage/all.info -q
	lcov --extract coverage/all.info $(shell pwd)/include/\* $(shell pwd)/source/\* --output-file coverage/coverage.info -q
	genhtml coverage/coverage.info --output-directory coverage/

-include $(DEPS)

