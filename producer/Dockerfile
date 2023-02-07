FROM gcc:latest

COPY etcdAPIs.hpp producer.cpp etcdAPIs.cpp /usr/src/producer/
#COPY producer.cpp /usr/src/producer/

WORKDIR /usr/src/producer

RUN mkdir -p /usr/src/producer/bin
RUN g++ -std=c++11 -c ./etcdAPIs.cpp
RUN g++ -std=c++11 -c ./producer.cpp

RUN g++ -o ./bin/producer etcdAPIs.o producer.o
#RUN g++ -o ./bin/producer producer.o

#CMD ["./bin/producer"]
