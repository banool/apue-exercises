# Exercises for Advanced Programming in the UNIX Environment

## Overview
This repo contains my answers for the exercises in Advanced Programming in the UNIX Environment (3rd edition) by W. Richard Stevens and Stephen A. Rago. 

## How this works
The answers, code or otherwise, are written in the markdown file `exercises.md`. The python script `to_code.py` then scans this file for code. When it finds a code block, it names it according to the exercise subheading, creates an individual file for the code block in the `exercises` directory, and then compiles it. The compiled files aren't included in the repo.

## Resources
The `apue.h` file is included in the exercises directory, a cross platform header file which defines a bunch of constants and function prototypes for the code in this repo. This file was provided by the authors of the book. The rest of the code in the book can be found [here](http://www.apuebook.com/code3e.html).

## Why am I doing this?
This book was one of the 6 that Facebook gave me to prepare for my upcoming internship with them as a production engineer. The other 5 books are:
- TCP/IP Illustrated Volume 1
- The Illustrated Network
- The Art of UNIX Programming
- Systems Performance: Enterprise and the Cloud
- Linux and the Unix Philosophy

It seems like this book has the greatest practical use in that is is concise, considers code, and has exercises. I have enjoyed *The Art of Unix Programming* as well.

