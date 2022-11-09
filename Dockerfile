FROM ubuntu:20.04

WORKDIR /do-you-even-bench-bro

RUN apt-get update
RUN apt-get install apt-utils -y
RUN apt-get install build-essential -y
RUN apt-get install make -y
RUN apt-get install manpages-dev -y
# RUN make lmbench

CMD ["bash"]