FROM gcc:latest

COPY etcdAPIs.hpp producer.cpp etcdAPIs.cpp /usr/src/producer/


RUN apt-get update && apt-get install -y curl

RUN export version=$(curl -sL https://dl.k8s.io/release/stable.txt) && \
    curl -L --remote-name-all https://dl.k8s.io/$version/bin/linux/amd64/{kubectl} && \
    chmod +x kubectl && \
    mv kubectl /usr/local/bin/ && \
    chmod +x /usr/local/bin/kubectl && \
    curl -L --remote-name https://github.com/etcd-io/etcd/releases/download/v3.5.1/etcd-v3.5.1-linux-amd64.tar.gz && \
    tar xzvf etcd-v3.5.1-linux-amd64.tar.gz && \
    mv etcd-v3.5.1-linux-amd64/etcdctl /usr/local/bin/

WORKDIR /usr/src/producer

RUN mkdir -p /usr/src/producer/bin
RUN g++ -std=c++11 -c ./etcdAPIs.cpp
RUN g++ -std=c++11 -c ./producer.cpp

RUN g++ -o ./bin/producer etcdAPIs.o producer.o
