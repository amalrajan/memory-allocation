# Use GCC base image
FROM gcc:latest

# Copy the source code into the container
COPY src /usr/src/memory

# Set the working directory
WORKDIR /usr/src/memory

# Compile the project using the Makefile
RUN make

# Default command to run tests
CMD ["make", "test"]
