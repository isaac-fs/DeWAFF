# Deceived Weighted Average Filters Framework

## Table of contents
- [Deceived Weighted Average Filters Framework](#deceived-weighted-average-filters-framework)
  - [Table of contents](#table-of-contents)
  - [Description](#description)
  - [Installation](#installation)
  - [Execution](#execution)
  - [Benchmark mode](#benchmark-mode)
  - [Documentation](#documentation)

## Description
Implementation of the image abstraction framework DeWAFF in C++. This framework allows to use WAF (Weighted Average Filters) with their input decoupled as a weigthing input and processing input. The weighthing input is used to generate the kernel values for the WAFF, and  the processing input serves as input for the filter. This tehcnique is known as "deceiving", hence the name of the framework. With this approach the filter's kernel is weighted with the original input but the actual input for the filter is the original image filtered through an UnSharp Mask. Normal WAFs use the same image for both processes. This is the novelty of this framework, to avoid a pipelined approach by applying two different techniques to an image, but instead combine them and apply them as one.

The DeWAFF is aimed at improving patter recognition tasks providing the following improvements:
- Noise reduction
- Irrelevant detail simplification
- Border emphasis of Regions Of Interest
- Enhanced separation of ROIs from the background

## Installation

This project requires that OpenCV is alredy installed in your machine. For this you can follow the [OpenCV linux installation guide](https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html).

- Clone the repository from https://github.com/isaac-fs/DeWAFF

```bash
    git clone https://github.com/isaac-fs/DeWAFF.git
```

- Build the project

```bash
    cd ./DeWAFF
    cmake .
    cmake --build .
```
After these steps binary named `DeWAFF` should appear in the directory. To recompile the project you can use the generated Makefille.

## Execution
To execute the framework run `./DeWAFF` accompanied of one or more of the following options:
```terminal
        --------------------------------------------------------------------
        | Options  | Description                                           |
        --------------------------------------------------------------------
        | -i       | Process an image given a file name -i <file name>     |
        | -v       | Process a video given a file name -v <file name>      |
        | -b < N > | Run a series of N benchmarks for a video or an image. |
        | -h       | Display this help message                             |
        --------------------------------------------------------------------
```

For example, to process an image the command looks like this
```bash
    ./DeWAFF -i /path/to/image/file
```
To process a video just change the flag from `-i` to `-v` as follows
```bash
    ./DeWAFF -v /path/to/video/file
```
These options will generate an output in the `/path/to/file` directory with the applied filter acronym as suffix `file_ACRONYM.extension`.

## Benchmark mode

There is also a benchmark mode that allows to run several executions over an image without saving it. This can be helpful to measure the performance of a particular DeWAF for your needs.

To benchmark an image pass the image with the `-i` flag, but this time add the benchmark flag `-b` to indicate the number of benchmarking runs. The number of runs has to be a positive integer equal or greater than one
```bash
    ./DeWAFF -i /path/to/image/file -b 10
```
To benchmar a video just change the flag from `-i` to `-v` as follows
```bash
    ./DeWAFF -v /path/to/video/file -b 3
```
Take into consideration that videos take a long time to benchmark as each frame has to be processed

## Documentation
The documentation for this project is hosted at https://isaac-fs.github.io/DeWAFF/