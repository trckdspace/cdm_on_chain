FROM ubuntu:22.04
RUN apt update && apt upgrade -y
RUN apt install build-essential git cmake -y
RUN git clone https://github.com/trckdspace/cdm_on_chain.git 
RUN cd cdm_on_chain/extern/perturb && \
    mkdir build && cd build && cmake .. && make && \
    cd .. && mkdir -p lib && mv build/libperturb.so lib 
RUN cd cdm_on_chain && mkdir build && cd build && cmake .. && make