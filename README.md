# armNeon

was tested on RPi4, which has AArch64 (v8-A), including NEON

## Build
./build.sh
creates build/ directory, runs for two targets.
First target is regular C-code, without NEON usage
Second target is using NEON Intrinsics
builded executables appear at build folder, named agc_test and agc_test_neon

## Run
./run.sh
simple run script which sets input arguments and runs programs
arguments:
1. input file
2. output file
3. number of repeats
4. verbose

number of repeats were included to provide "weightened" estimation of time

## Output
after execution code prints elapsed time, assert result and  difference in values (if they are present)


## Summary
Right now it looks like this (100 repeats, no verbose):

### executing regular ###

I don't use NEON

PASSED

time taken for 100 repeats: 856.667ms

### executing neon ###

I was built with NEON usage

FAILED

time taken for 100 repeats: 168.329ms


FAILED in NEON variant happens because of agressive float-point optimizations
verbosed output shows exact difference in values:

iteration: 43458 expected: 14184 received: 14183

iteration: 43533 expected: -14184 received: -14183

so here are two iterations with fails and I assume this as future work
