#!/usr/bin/zsh

yes | for eps in *.svg; do; svg2eps $eps; done;
