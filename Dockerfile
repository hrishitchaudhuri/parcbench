FROM ubuntu:20.04

COPY lmbench-3.0-a9 /lmbench-3.0-a9
WORKDIR /lmbench-3.0-a9/src

RUN apt-get update
RUN apt-get install apt-utils -y
RUN apt-get install build-essential -y
RUN apt-get install make -y
RUN apt-get install manpages-dev -y
RUN make lmbench

CMD ["bash"]