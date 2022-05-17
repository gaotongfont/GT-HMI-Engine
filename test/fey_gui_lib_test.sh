#!/usr/bash

rm -vr ../../gui-test/gui/*

cp -v ../build/lib/libgt_gui_static.a -t ../../gui-test/gui/

cp -vrp ../src/ -t ../../gui-test/gui/
rm -vrf ../../gui-test/gui/*/*/*.c
rm -vrf ../../gui-test/gui/*/*.txt
