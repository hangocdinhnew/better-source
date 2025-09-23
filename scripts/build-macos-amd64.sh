#!/bin/sh

git submodule init && git submodule update

brew install sdl3

./waf configure -T debug --disable-warns $* &&
./waf build
