FROM aite:base

COPY . /usr/src/projects/aite
WORKDIR /usr/src/projects/aite/build

RUN cmake ..
RUN make -j 4

WORKDIR /usr/src/projects/aite/build_coverage

RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage -j 4