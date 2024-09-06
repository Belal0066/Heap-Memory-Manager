# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RESET='\033[0m'

run_test() {
    local command="$1"
    local description="$2"
    local success_msg="$3"
    local failure_msg="$4"

    echo  "\n${YELLOW}Running ${description}...${RESET}"
    if $command | tee /dev/stderr | grep -q 'null'; then
        echo  "${RED} ✗ ${failure_msg}${RESET}"
    else
        echo  "${GREEN} ✓ ${success_msg}${RESET}"
    fi
}

# Test cases
run_test "./_test_hmm 1000 128" "basic functionality test" "Test passed" "Test failed"
run_test "./_test_hmm 10 100000" "large block allocations" "Test passed" "Test failed"
run_test "./_test_hmm 1000 0" "zero block size test" "Test passed" "Test failed"
run_test "./_test_hmm 1000 128 2 500 1000" "allocation with non-uniform freeing" "Test passed" "Test failed"
run_test "./_test_hmm 1000 128 10 1 1000" "freeing with large steps" "Test passed" "Test failed"
run_test "./_test_hmm 1000 128 1 1 1" "minimal range freeing" "Test passed" "Test failed"
run_test "./_test_hmm 1000 128 1 1000 1000" "maximal range freeing" "Test passed" "Test failed"
run_test "./_test_hmm 100000 64 1 1 100000" "full freeing stress test" "Test passed" "Test failed"
run_test "./_test_hmm 50000 128 3 1 50000" "high fragmentation stress test" "Test passed" "Test failed"
run_test "./_test_hmm 1000 4096 1 1 1000" "exact program break lowering" "Test passed" "Test failed"

run_test "./_test_hmm 1 32768" "boundary allocation and free" "Test passed" "Test failed"
run_test "./_test_hmm 10 1048544" "memory exhaustion test" "Test passed" "Test failed"

echo  "\n${YELLOW}All _test_hmm tests completed.${RESET}"

