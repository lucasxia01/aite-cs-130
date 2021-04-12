### Build/test container ###
# Define builder stage
FROM aite:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:latest as deploy

# Copy server output binary and config file to "." 
  COPY --from=builder /usr/src/project/build/bin/server .
  COPY --from=builder /usr/src/project/server_config .

# Exposes the port but doesn't publish it
  EXPOSE 80

# specify the binary name to run when container starts
  ENTRYPOINT ["./server"]

# Use CMD to specify arguments to entrypoint (config file)
  CMD ["server_config"]
