FROM aite:base

COPY . /usr/src/project
WORKDIR /usr/src/project/build_coverage

RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage