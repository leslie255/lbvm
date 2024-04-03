import sys
import subprocess

args = iter(sys.argv)

arg0 = next(args)

def printHelp() -> None:
	print("usage: {} [--dbg] [INPUT.asm]".format(arg0))

def main() -> None:
    inputPath: str = None
    dbg = False
    for arg in args:
        if arg == "--help":
            printHelp()
            exit(0)
        elif arg == "--dbg":
            dbg = True
        else:
            if inputPath!= None:
                print("multiple input paths are not supported")
                printHelp()
                exit(1)
            inputPath = arg

    print("--- Running `cabal build` @ ./lbvm_asm")
    subprocess.run(["cabal", "build"], cwd="lbvm_asm")

    inputPath_ = ("../" + inputPath)
    print("--- Running `cabal run exes -- {}` @ ./lbvm_asm".format(inputPath))
    result = subprocess.run(["cabal", "run", "exes", "--", ("../" + inputPath)], cwd="lbvm_asm", stdout=subprocess.PIPE).stdout.decode()

    print("--- Writing to code.h")
    codeHFile = open("code.h", "w+")
    codeHFile.write(result)
    codeHFile.close()
    if dbg: # too noisy, keep it behind debug flag
        subprocess.run(["bat", "-l", "c", "code.h"])

    print("--- Running `make all`")
    subprocess.run(["make", "all"])

    print("--- Running `bin/lbvm`")
    if dbg:
        subprocess.run(["bin/lbvm", "--dbg"])
    else:
        subprocess.run(["bin/lbvm"])

if __name__ == "__main__":
    main()