Import("env")
import subprocess

version = "undefined"

defFile = open("src/definitions.h")
for line in defFile:
    if line.startswith("#define MAJOR_VERSION"):
        version = line.replace("#define MAJOR_VERSION ", "")
        version = version.strip()
defFile.close()

program_file_name = "blecker_v" + version 

env.Replace(PROGNAME=program_file_name)
