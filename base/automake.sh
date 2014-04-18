#!/bin/bash

gcc -export-dynamic mui.c -o mui `pkg-config --cflags --libs gtk+-2.0` `pkg-config --cflags --libs libglade-2.0`

