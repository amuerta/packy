# UNIVERSITY OF KUNCA - Teory of Information and Encoding Course Project
## Features
- lab1
  + utf-8 encoding and decoding
  + basic Shannon-Fano codding
  + basic Huffman codding
- lab2
  + simple parity check for error detection 
  + Hamming codes
  + basic LZ78 compression algorithm
- lab3
  + Modeling an analog signal, PCM (Pulse Code Modulation)

## How to get started
### Normal Route
- install clang, make, git
- clone the repo
- navigate to whatever example you want to run: `cd <repo>/lab<N>_1-3/lab<LAB_N>/`
- run: `make test`
### NIX build system
in first task of first laboratory there is nix enviroment which should be enough to run
all examples
navigate there and enter development shell by running `nix develop`
then actions are the same as in Normal route.

## WARNING
- all the code here is very amateur and was done in short time window, before trying to use it, get familiar with the topic to avoid misunderstaing or false confidance.
- C was used as base language, but in a dirt way. Don`t attempt to use tricks from this repo with serious intent if you are learning things like me.
