FROM ubuntu:20.04

WORKDIR /do-you-even-bench-bro

RUN apt-get update
RUN apt-get install apt-utils -y
RUN apt-get install build-essential -y
RUN apt-get install make -y
RUN apt-get install manpages-dev -y
RUN apt-get install -y python3
RUN apt-get install -y python3-pip
RUN python3 -m pip install --no-cache-dir --upgrade pip
# RUN make lmbench

CMD ["bash"]