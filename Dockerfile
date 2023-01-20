FROM jncsw/llvm12.0.1:latest
COPY ./SmallRace-Detector /SmallRace-Detector
COPY ./SmallRace-Generator /SmallRace-Generator
RUN apt-get -qq update && apt-get -qq -y install libicu-dev libxml2-dev python3-pip
RUN pip3 install conan
RUN cd /SmallRace-Generator && mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_INSTALL=/llvm-project/ ../ && make -j4
RUN cd /SmallRace-Detector && mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_INSTALL=/llvm-project/build ../ && make -j4
RUN cd /SmallRace-Generator/build/bin/ && ./st-racedetect /SmallRace-Generator/examples/ --dump-ir
RUN /SmallRace-Detector/build/bin/openrace /SmallRace-Generator/build/bin/t.ll



