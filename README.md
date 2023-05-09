# Deceived Weighted Average Filters Framework

## Table of contents
- [Deceived Weighted Average Filters Framework](#deceived-weighted-average-filters-framework)
	- [Table of contents](#table-of-contents)
	- [Description](#description)
	- [Documentation](#documentation)
	- [Installation](#installation)
	- [Execution](#execution)
	- [Benchmark mode](#benchmark-mode)

## Description
Implementation of the image abstraction framework *DeWAFF* in C++. This framework allows to use WAF (Weighted Average Filters) with their input decoupled in to a weigthing input and a processing input. The weighthing input is used to generate the kernel values for the WAF and the processing input serves as input for the filter. This tehcnique is known as "deceiving", hence the name of the framework. With this approach the filter's kernel is weighted with the original input and as input it takes the original input filtered through an UnSharp Mask filter (Laplacian deceive). Normal WAFs use the same image for both processes. This is the novelty of this framework, to avoid a pipelined approach by applying two different techniques to an image, but instead combine them and apply them as one.

The *DeWAFF* is aimed at improving patter recognition tasks providing the following improvements:
- Noise reduction
- Irrelevant detail simplification
- Border emphasis of Regions Of Interest
- Enhanced separation of ROIs from the background

Before DeWAFF             |  After DeWAFF
:-------------------------:|:-------------------------:
![Original image of a cancer cell using DeWAFF](/include/original.jpg "Original")  |  ![Processed image of a cancer cell using DeWAFF](/include/original_DBF_2.jpg "DBF")
[Images retrieved from S. S. Martin, Microtentacles and Metastasis, National Cancer Institute Univ. of Maryland Greenebaum Cancer Center, july 2009.](https://visualsonline.cancer.gov/details.cfm?imageid=10597)

## Documentation
The documentation for this project is hosted at https://isaac-fs.github.io/DeWAFF/. Most of the mathematical formulations for framework filters are found in the **Classes** section.

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
To execute the framework use the `./DeWAFF` command it can be as simple as to use
```bash
    ./DeWAFF -i path/to/image
```
But it supports a varied array of customization options, so a command can look like this
```bash
    ./DeWAFF --image path/to/file --filter dgf --parameters ws=31,rs=15,ss=10 --benchmark 20 --quiet
```

These are the full program instructions:
```terminal
usage: ./DeWAFF [-i | --image <file name>] | [-v | --video <file name>]
		[-f | --filter <filter type>]
		[-p | --parameters <filter parameters>]
		[-b | --benchmark <number of iterations>] [-h | --help]

	DEFAULT PARAMETERS
	- Filter:            dbf (Deceived Bilateral Filter)
	- Window size:       3
	- Neighborhood size: 3
	- Range Sigma:       1.0
	- Spatial Sigma:     1.0
	- USM Lambda:        1.0

	PROGRAM OPTIONS
	-i, --image: Process an image given a file name. The file name goes
	after the option.
	Example: '-i picture.png'

	-v, --video: Process a video given a file name. The file name goes
	after the option.
	Example: '-v video.mp4'

	-f, --filter: Choose which filter to use. The availabe options are:
		- dbf:  deceived bilateral filter
		- dsbf: deceived scaled bilateral filter
		- dnlmf:deceived non local means filter
		- dgf:  deceived guided filter
	For example, to process an image using the deceived bilateral filter
	use: './DeWAFF -i image.png -f dbf'.

	-p, --parameters: Change the filter parameters. Available parameters:
		- ws:    Window size
		- rs:    Range Sigma
		- ss:    Spatial Sigma
		- lambda:Lambda value for the Laplacian deceive
		- ns:    Neighborhood size for the DNLM filter
	It is possible to change one or more parameters in the same line,
	for example '-p ws=15,rs=10,ss=10' would change the window size and
	the range and spatial sigma values for the filter. Using just
	'-p ws=15' would only change its window size.
	The 'ns' option only works with the filter set to 'dnlm'.
	If 'lambda=0' the Laplacian the deceive will be disabled.

	-b, --benchmark: Run a series of N benchmarks for a video or an image.
	This option will run aseries of N benchmarks and
	display the results in the terminal.
	Note: The results are NOT saved during this process.
	Indicate the number of iterations after the flag,
	for example '-b 10' would indicate to run the filter
	ten separate times.

	-q, --quiet: Run in quiet mode. Does not displays the file and
	filter information.

	-h, --help: Display the program's help message. The long version
	--help shows the full program's help
```

The output wil be generated in the `/path/to/file` directory with the applied filter acronym as suffix `file_ACRONYM.extension`.

## Benchmark mode

The benchmark mode that allows to run several executions over an image without saving it. This can be helpful to measure the performance of a particular DeWAF for your needs.

To benchmark an image pass the image with the `-i` flag, but this time add the benchmark flag `-b` to indicate the number of benchmarking runs. The number of runs has to be a positive integer equal or greater than one
```bash
    ./DeWAFF -i /path/to/image/file -b 10
```
To benchmark a video just change the flag from `-i` to `-v` as follows
```bash
    ./DeWAFF -v /path/to/video/file -b 3
```
Take into consideration that videos take a long time to benchmark as *each frame* has to be processed!

This project was made in collaboration with the PRIS Lab (https://pris.eie.ucr.ac.cr/) from the University of Costa Rica for my graduation project.
