### Build/test container ###
# Define builder stage
FROM aite:base as builder

# Share work directory
COPY . /usr/src/projects/aite
WORKDIR /usr/src/projects/aite/build

# Build and test
RUN cmake ..
RUN make -j 2
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:latest as deploy

# Copy server output binary and config file to "." 
  COPY --from=builder /usr/src/projects/aite/build/bin/server .
  COPY --from=builder /usr/src/projects/aite/server_config .
  COPY --from=builder /usr/src/projects/aite/static_files/ ./static_files/
  COPY --from=builder /usr/src/projects/aite/memes/ ./memes/


# Exposes the port but doesn't publish it
  EXPOSE 80

# specify the binary name to run when container starts
  ENTRYPOINT ["./server"]

# Use CMD to specify arguments to entrypoint (config file)
  CMD ["8", "server_config"]
