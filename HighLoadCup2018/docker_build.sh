#!/bin/sh

docker build -t main .
docker login stor.highloadcup.ru
docker tag main stor.highloadcup.ru/accounts/careful_antelope
docker push stor.highloadcup.ru/accounts/careful_antelope
