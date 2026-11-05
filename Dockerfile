FROM gcc:16.2.0

# Install gdb and any other debugging tools you might need
RUN apt-get update && apt-get install -y gdb iputils-ping dnsutils

WORKDIR /app

# Copy all source files and Makefile into the container
COPY . /app

# Compile the C modules into the final binary
RUN make