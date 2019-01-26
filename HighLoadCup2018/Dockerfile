FROM centos:latest

WORKDIR /root

ADD highloadcup2018 .

RUN yum install -y unzip
RUN mkdir data

EXPOSE 80

CMD unzip -qq /tmp/data/data.zip -d data && cp /tmp/data/options.txt data/options.txt && ./highloadcup2018 data
