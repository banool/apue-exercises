#!/usr/local/bin/python3

"""
This script exists specifically for the exercises in Advanced Programming
in the UNIX Environment (3rd Edition) by W. Richard Stevens and Stephen A. Rago.

The script reads a target markdown file, produces files with the C code in it, 
and compiles it. See the exercises.md file for an example of how it should be 
formatted. If it runs into code under a subheading (## ) it will make the code 
with the name of the exercise. E.g. If the subheading is 3.4 and code is found, 
it will write it to 03.04.c and compile it to 03.04

Note that it only expects there to be one code segment per exercise, as this is
how it seems like the exercises are organised in the book.

Make sure to have a newline at the end of your target file (exercises.md).
Otherwise, it might miss code that runs to the end of the file.

This script can also take "clean" in the args to remove all the compiled files.
"""

import os
from sys import argv

marker_exercise = "## "
marker_code = "```"

source = "exercises.md"
targetDir = "exercises"

clean = False

def main():
    os.system("mkdir -p " + targetDir)

    with open(source, "r") as f:
        content = f.read().splitlines()

    currentExercise = "0.0"
    inCode = False
    code = []
    for line in content:
        start = line[:3]

        # This indicates the start or end of code.
        if start == marker_code:
            inCode = not inCode
            continue

        # We're in code, so store it in a list of lines.
        if inCode:
            code.append(line)
            continue

        # We've left a code block, write to file.
        if not inCode and len(code) > 0:
            compiledFName = os.path.join(targetDir, currentExercise)
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

        # Don't worry about ==, ignore those.
        # We ignore every other type of line from this point really
        # except for when we're in code, in which case we store it.

def writeCode(code, codeFName, compiledFName):
    # Check if the code already exists and if anything is different.
    # If it does and nothing has changed, no need to rewrite/recompile.
    if existsUnchanged(code, codeFName):
        print("{} unchanged".format(compiledFName.split("/")[-1]))
        return
    else:
        print("{} changed".format(compiledFName.split("/")[-1]))

    # Otherwise we write anew and compile.
    with open(codeFName, "w") as f:
        for i in code:
            f.write("{}\n".format(i))
    # Compile the code.
    command = """ gcc "{}" -o "{}" """.format(codeFName, compiledFName)
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
    main()
