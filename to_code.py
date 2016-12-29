#!/usr/bin/env python3

"""
This script exists specifically for the exercises in Advanced Programming
in the UNIX Environment (3rd Edition) by W. Richard Stevens and Stephen A. Rago.

The script reads a target markdown file, produces files with the C code in it, 
and compiles it. See the exercises.md file for an example of how it should be 
formatted. If it runs into code under a subheading (## ) it will make the code 
with the name of the exercise. E.g. If the subheading is 3.4 and code is found, 
it will write it to 03.04.c and compile it to 03.04

Note that it only expects there to be one code segment per exercise, as this is
how it seems like the exercises are organised in the book. Update: If there is
more than one code snippet in an exercise, it will now number them like this:
08.01, 08.01.2, 08.01.3, etc.

This script can take "clean" in the args to remove all the compiled files.
This script can take "compile" in the args to force compile all the files, even
those that have no changes in their code.

You can include NO after ``` in order to prevent it being written/compiled.
"""

import os
from sys import argv

marker_exercise = "## "
marker_code = "```"
marker_nocompile = "NO"

source = "exercises.md"
targetDir = "code"
libDir = "../lib"

clean = False
forceCompile = False

def main():
    os.system("mkdir -p " + targetDir)

    with open(source, "r") as f:
        content = f.read().splitlines()

    content.append("") # So you don't have to worry about the newline.

    currentExercise = "0.0"
    numSnippetsInExercise = 0
    inCode = False
    noCompile = False
    code = []
    for line in content:
        start = line[:3]
        lang  = line[3:]

        # This indicates the start or end of code.
        if start == marker_code:
            if marker_nocompile in lang.upper():
                noCompile = True
                numSnippetsInExercise -= 1
            if not inCode:
                numSnippetsInExercise += 1
            inCode = not inCode
            continue

        # We're in code, so store it in a list of lines.
        if inCode:
            code.append(line)
            continue

        # We've left a code block, write to file.
        if not inCode and len(code) > 0:
            # If we hit ``` and noCompile is True, turn ignore this code.
            if noCompile:
                noCompile = False
                code = []
                continue
            compiledFName = os.path.join(targetDir, currentExercise)
            if numSnippetsInExercise > 1:
                compiledFName = compiledFName + "." + str(numSnippetsInExercise)
            codeFName = compiledFName + ".c"
            # If clean=True, we remove compiledFname
            if clean:
                # Use -f so it doesn't complain if the file doesn't exist.
                os.system("""rm -f "{}" """.format(compiledFName))
            else:
                # Otherwise, write the lines we've collected to file and compile.
                writeCode(code, codeFName, compiledFName)
            # Empty the collected lines of code.
            code = []
            continue

        # This indicates the start of an exercise.
        if start == marker_exercise:
            # We expect the exercise like 3.2 or 13.11
            exercise = line.split()[1]
            major, minor = exercise.split(".")
            currentExercise = major.zfill(2) + "." + minor.zfill(2)
            numSnippetsInExercise = 0

        # Don't worry about ==, ignore those.
        # We ignore every other type of line from this point really
        # except for when we're in code, in which case we store it.

def writeCode(code, codeFName, compiledFName):
    spacing = "  "
    compiledFnameEnd = compiledFName.split("/")[-1]
    if compiledFnameEnd.count(".") > 1:
        spacing = "" 
    # Check if the code already exists and if anything is different.
    # If it does and nothing has changed, no need to rewrite/recompile.
    if existsUnchanged(code, codeFName) and not forceCompile:
        print("{} {}unchanged".format(compiledFnameEnd, spacing))
        return
    else:
        print("{} {}changed".format(compiledFnameEnd, spacing))

    # Otherwise we write anew and compile.
    with open(codeFName, "w") as f:
        for i in code:
            f.write("{}\n".format(i))

    # Check for user headers to be included. This is pretty primitive.
    includes = []
    for line in code:
        name = None
        # Looking for #include "myheader.h" for example.
        if line[:10] == "#include \"":
            name = line[10:-3]
        if line[:13] == "//-#include \"":
            name = line[13:-3]
        if name:
            if name != "apue":
                path = os.path.join(libDir, name)
                includes.append(""""{}.c" """.format(path))

    # Constructing the string for the additional files with which to compile.
    from operator import add as op_add
    from functools import reduce
    # Just using this for fun to relive the Haskell days, an explicit
    # for loop would be more sensible in practice.
    includes = reduce(op_add, includes, "")

    # Compile the code.
    args = (libDir, codeFName, includes, compiledFName)
    command = """gcc -Wall -pthread -std=c99 -g -I{} -D_GNU_SOURCE "{}" {} -o "{}" -lrt""".format(*args)
    print(command)
    os.system(command)

def existsUnchanged(code, codeFName):
    # Check if the code already exists and if anything is different.
    try:
        with open(codeFName, "r") as f:
            curr = f.read().splitlines()
    except FileNotFoundError:
        return False

    if len(curr) != len(code):
        return False

    for lines in zip(curr,code):
        if lines[0] != lines[1]:
            return False

    return True

if __name__ == "__main__":
    if len(argv) > 1:
        if argv[1] == "clean" or argv[1] == "--clean":
            clean = True
        elif argv[1] == "compile" or argv[1] == "--compile":
            forceCompile = True
    main()
