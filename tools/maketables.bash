#!/bin/bash

go run maketables.go -local
cp unicode_private_tables.inl ../src
cp unicode_public_tables.cc ../src
cp _unicode_tables.hh ../src/zbs
