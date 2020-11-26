#!/bin/bash

if [ ! -d "App" ]; then
  mkdir App
fi

protoc --proto_path=./Proto \
       --cpp_out=./App \
       ./Proto/*.proto
