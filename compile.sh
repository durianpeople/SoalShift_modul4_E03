#!/bin/bash

gcc -Wall `pkg-config fuse --cflags` ./afs.c -o afs `pkg-config fuse --libs`
