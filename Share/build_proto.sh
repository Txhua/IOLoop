#!/bin/bash

protoc --proto_path=./Proto \
       --cpp_out=./App \
       ./Proto/*.proto
