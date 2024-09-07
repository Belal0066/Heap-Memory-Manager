CC = gcc
CFLAGS = -g

TARGET_HMM = ./tests/_test_hmm
TARGET_STRESS = ./tests/_test_stress
TARGET_MAIN = ./src/hmmMain

SOURCES_HMM = ./src/hmm.c ./tests/_test_hmm.c
SOURCES_STRESS = ./src/hmm.c ./tests/_test_stress.c
SOURCES_MAIN = ./src/hmm.c ./src/hmmMain.c

OBJECTS_HMM = $(SOURCES_HMM:.c=.o)
OBJECTS_STRESS = $(SOURCES_STRESS:.c=.o)
OBJECTS_MAIN = $(SOURCES_MAIN:.c=.o)

RM = rm -f

# Color codes
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[0;33m
RESET = \033[0m

# Default target
all: $(TARGET_HMM) $(TARGET_STRESS) $(TARGET_MAIN)

# Compile the _test_hmm target
$(TARGET_HMM): $(OBJECTS_HMM)
	$(CC) $(CFLAGS) -o $(TARGET_HMM) $(OBJECTS_HMM)

# Compile the _test_stress target
$(TARGET_STRESS): $(OBJECTS_STRESS)
	$(CC) $(CFLAGS) -o $(TARGET_STRESS) $(OBJECTS_STRESS)

# Compile the hmmMain target
$(TARGET_MAIN): $(OBJECTS_MAIN)
	$(CC) $(CFLAGS) -o $(TARGET_MAIN) $(OBJECTS_MAIN)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run tests for _test_hmm
testHmm: $(TARGET_HMM)
	@chmod +x ./tests/testScript.sh
	@./tests/testScript.sh

# Custom parameters for _test_hmm
customTestHmm: $(TARGET_HMM)
	@echo  "\n${YELLOW}Running _test_hmm with custom parameters: $(args)...${RESET}"
	@./$(TARGET_HMM) $(args)

# Run tests for _test_stress
testStress: $(TARGET_STRESS)
	@echo  "\n${YELLOW}Running stress tests with _test_stress...${RESET}"
	@./$(TARGET_STRESS) | tee /dev/stderr | (grep -q 'failed' && echo  "${RED} ✗ Stress test failed${RESET}" || echo  "${GREEN} ✓ Stress test passed${RESET}")

# Run hmmMain
runMain: $(TARGET_MAIN)
	@echo  "\n${YELLOW}Running hmmMain...${RESET}"
	@./$(TARGET_MAIN)

clean:
	$(RM) $(TARGET_HMM) $(TARGET_STRESS) $(TARGET_MAIN) $(OBJECTS_HMM) $(OBJECTS_STRESS) $(OBJECTS_MAIN)

.PHONY: all clean testHmm testStress runMain