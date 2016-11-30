# Exercises for Advanced Programming in the UNIX Environment

## Overview
This repo contains my answers for the exercises in Advanced Programming in the UNIX Environment (3rd edition) by W. Richard Stevens and Stephen A. Rago. 

## How this works
The answers, code or otherwise, are written in the markdown file `exercises.md`. The python script `convert_to_code.py` then scans this file for code. When it finds a code block, it names it according to the exercise subheading, creates an individual file for the code block in the `exercises` directory, and then compiles it. The compiled files aren't included in the repo.

## Resources
The `apue.h` file is included in the exercises directory, a cross platform header file which defines a bunch of constants and function prototypes for the code in this repo. This file was provided by the authors of the book. The rest of the code in the book can be found [here](http://www.apuebook.com/code3e.html).
