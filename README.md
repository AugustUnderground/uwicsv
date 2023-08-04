# UWI CSV

Really slow and crappy CSV format for Spectre Unified Waveform Interface (UWI).

**Note:** Only works for `ac`, `dc`, `tran` and `noise` analyses :frowning_face:

## Build

Adjust `INC` path in `Makefile`, make sure the path to `uwi.h` is for the right
`spectre` version you are using!

```Makefile
INC    = -I/path/to/uwi/include     # <- Adjust this
```

To build the shared object, just

```sh
$ make
```

This will compile to `libuwicsv.so` containing the `CSV` uwi format.

## Use

```sh
$ spectre -64 -format uwi -uwifmt CSV -uwilib ./libuwicsv.so ./example/input.scs
```

This will create a `input.raw/` directory, containing the `*.csv` files for
each analysis.
