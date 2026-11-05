FROM gcc:latest

WORKDIR /app

# Copy all source files and Makefile into the container
COPY . /app

# Compile the C modules into the final binary
RUN make