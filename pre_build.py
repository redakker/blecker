Import("env")
import subprocess

version = "undefined"
revision = "rev_NO_GIT"
boardName = "blecker"

try:

    revision = (
        subprocess.check_output(["git", "rev-list", "--count", "HEAD"])
        .strip()
        .decode("utf-8")
    )

except:
    print ("No Git installed. Version number will be skipped in a filename")

defFile = open("src/definitions.h")
for line in defFile:
    if line.startswith("#define MAJOR_VERSION"):
        version = line.replace("#define MAJOR_VERSION ", "")
        version = version.strip()
defFile.close()


defFile = open("src/definitions.h")
for line in defFile:
    if line.startswith("#define MAJOR_VERSION"):
        version = line.replace("#define MAJOR_VERSION ", "")
        version = version.strip()
    if line.startswith("#define BOARD_NAME"):
        boardName = line.replace("#define BOARD_NAME ", "").replace("\"","").replace('  ', '').replace('\t', '').replace('\n\n', '\n').replace('\n', '')
        boardname = version.strip()
defFile.close()

program_file_name = boardName + "_v" + version + "-" + revision

env.Replace(PROGNAME=program_file_name)
