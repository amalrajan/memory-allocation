# Memory Management Simulation

## Overview

This repository contains a set of C files that simulate various memory management strategies in a custom memory allocation framework. It includes a memory allocator, a test runner, and a set of memory tests.

### Memory Management Strategies

1. First-Fit: Allocates the first block of memory that is large enough to meet the request.

2. Best-Fit: Searches the entire list of blocks and chooses the smallest block that is adequate to fulfill the request.

3. Worst-Fit: Opposite of best-fit, this technique selects the largest available block, aiming to leave the largest remaining part possible after allocation.

4. Next-Fit: Similar to first-fit, but instead of starting the search from the beginning each time, it starts from where it left off last time.

### Files

- `mymem.c`: Implements the memory allocation and deallocation functions. It supports different strategies like best-fit, worst-fit, first-fit, and next-fit.
- `testrunner.c`: A framework for running tests. It includes functions to execute tests within specified time limits and handle timeouts.
- `memorytests.c`: Contains various tests for the memory management system, including stress tests and tests for sequential and random allocations.

## Running Tests

The tests are authored by [L. Angrave](https://siebelschool.illinois.edu/about/people/faculty/angrave).

To compile and run the tests, use the following commands:

```bash
make tests
```

or

```bash
gcc -o memtest mymem.c testrunner.c memorytests.c
./memtest -test <test_name> <strategy>
```

## Test a specific strategy

To test a specific strategy, use the following command:

```bash
./mem -test <test_name> <strategy>
```

where `<test_name>` is the name of the test to run (e.g., `stress`, `sequential`, `random`) and `<strategy>` is the memory allocation strategy to use (e.g., `first`, `best`, `worst`, `next`).

## Run as a docker container

To run the tests in a docker container, use the following commands:

```bash
docker build -t memory-management-tests .
docker run -it --name test-container memory-management-tests
```

This will build the docker image and run the tests inside the container.
