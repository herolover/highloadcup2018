FROM centos:7

WORKDIR /root

ADD highloadcup2018 .

RUN pacman -S --noconfirm unzip
RUN mkdir data

EXPOSE 80

CMD unzip -qq /tmp/data/data.zip -d data && ./highloadcup2018 data
