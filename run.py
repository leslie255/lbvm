import sys
import subprocess

args = iter(sys.argv)

arg0 = next(args)

def printHelp() -> None:
	print("usage: {} [INPUT.asm]".format(arg0))

def main() -> None:
    inputPath: str
    try:
        arg1 = next(args)
        if arg1 == "--help":
            printHelp()
            exit(1)
        else:
            inputPath = arg1
    except StopIteration:
        print("Expect one argument as input file")
        printHelp()
        exit(1)

    print("--- Running `cabal build` @ ./lbvm_asm")
    subprocess.run(["cabal", "build"], cwd="lbvm_asm")

    inputPath_ = ("../" + inputPath)
    print("--- Running `cabal build run exes -- {}` @ ./lbvm_asm".format(inputPath))
    result = subprocess.run(["cabal", "run", "exes", "--", ("../" + inputPath)], cwd="lbvm_asm", stdout=subprocess.PIPE).stdout.decode()
    print("{}".format(result), end="")

    print("--- Writing to code.h")
    codeHFile = open("code.h", "w+")
    codeHFile.write(result)
    codeHFile.close()

    print("--- Running `make all`")
    subprocess.run(["make", "all"])

    print("--- Running `bin/lbvm`")
    subprocess.run(["bin/lbvm"])

if __name__ == "__main__":
    main()