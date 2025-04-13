# PVS-Studio for Linux

PVS-Studio Installation
==================================
PVS-Studio is distributed as an archive named pvs-studio-version.arch.tgz. To install it, you need to run the following commands:

````
$ tar -xzf pvs-studio-*.tgz
$ sudo ./install.sh
````

These commands will perform unpacking of the archive into the current directory and application installation to /usr/bin.
The distribution package contains the following files:
- pvs-studio - the analyzer kernel
- pvs-studio-analyzer - a script to run project analysis
- plog-converter - a utility for converting the analysis log into different formats
- plog-converter-source.tgz - plog-converter source files

Quick run
==================================
The best way to use the analyzer is to integrate it with your build system, namely near the compiler call. However, if you want to run the analyzer for a quick test on a small project, use the pvs-studio-analyzer utility.
This utility requires an installed Perl interpreter and the strace utility to work properly.

Build the project using the following command:
```
$ pvs-studio-analyzer trace -- make
```

You can use any other build command with all the necessary parameters instead of make, for example:
```
$ pvs-studio-analyzer trace -- make CXX=g++ -j2
```

Once the project is built, run the following command:
```
$ pvs-studio-analyzer analyze -l /path/to/PVS-Studio.lic -o PVS-Studio.log
```

The analyzer warnings will be saved in the specified PVS-Studio.log file.
