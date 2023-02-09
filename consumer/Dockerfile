FROM gcc:latest

COPY etcdAPIs.hpp consumer.cpp etcdAPIs.cpp /usr/src/consumer/
#COPY producer.cpp /usr/src/producer/

WORKDIR /usr/src/consumer

RUN mkdir -p /usr/src/consumer/bin
RUN g++ -std=c++11 -c ./etcdAPIs.cpp
RUN g++ -std=c++11 -c ./consumer.cpp

RUN g++ -o ./bin/consumer etcdAPIs.o consumer.o
#RUN g++ -o ./bin/producer producer.o

# CMD ["./bin/consumer"]
