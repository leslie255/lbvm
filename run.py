import sys
import os
import subprocess


def printHelp(arg0: str) -> None:
    print("usage: {} [--dbg] [INPUT.asm]".format(arg0))


def clone_submodules() -> None:
    def clone_submodules() -> None:
        print("--- Running `git submodule init`")
        subprocess.run(["git", "submodule", "init"])
        print("--- Running `git submodule update --remote`")
        subprocess.run(["git", "submodule", "update", "--remote"])

    if not os.path.isdir("lbvm_asm"):
        clone_submodules()
    elif len(os.listdir("lbvm_asm")) == 0:
        clone_submodules()


def make_bin_dir() -> None:
    if not os.path.isdir("bin"):
        print("--- Making `bin` directory")
        os.mkdir("bin")


def cmd_exists(cmd: str) -> bool:
    try:
        result = subprocess.run(["which", cmd], stdout=subprocess.PIPE)
        return result.returncode == 0
    except:
        return False


def check_requirements(arg0: str) -> None:
    if os.name != "posix":
        print("{} requires UNIX-like systems".format(arg0))
        exit(1)

    def require_cmd(cmd: str, notes: str = None) -> None:
        notes_ = "" if notes == None else notes
        if not cmd_exists(cmd):
            print("requires `{}`".format(cmd, notes_))
            exit(1)

    require_cmd("git")
    require_cmd("make")
    require_cmd("clang")
    require_cmd(
        "cabal", notes="(install GHCup and cabal at https://www.haskell.org/ghcup/)"
    )


def main() -> None:
    args = iter(sys.argv)
    arg0 = next(args)
    check_requirements(arg0)

    inputPath: str = None
    dbg = False
    for arg in args:
        if arg == "--help":
            printHelp(arg0)
            exit(0)
        elif arg == "--dbg":
            dbg = True
        else:
            if inputPath != None:
                print("multiple input paths are not supported")
                printHelp(arg0)
                exit(1)
            inputPath = arg

    if inputPath == None:
        print("expects one argument for input file")
        printHelp(arg0)
        exit(1)

    clone_submodules()
    make_bin_dir()

    print("--- Running `cabal build` @ ./lbvm_asm")
    result = subprocess.run(["cabal", "build"], cwd="lbvm_asm")
    if result.returncode != 0:
        print(
            "--- `cabal run` exited with non zero code, try deleting `lbvm_asm/` and then run {} again".format(
                arg0
            )
        )
        exit(1)

    inputPath_ = "../" + inputPath
    print("--- Running `cabal run exes -- {}` @ ./lbvm_asm".format(inputPath_))
    result = subprocess.run(
        ["cabal", "run", "exes", "--", inputPath_],
        cwd="lbvm_asm",
        stdout=subprocess.PIPE,
        check=True,
    ).stdout.decode()

    print("--- Writing to code.h")
    codeHFile = open("code.h", "w+")
    codeHFile.write(result)
    codeHFile.close()
    if dbg:  # too noisy, keep it behind debug flag
        subprocess.run(["bat", "-l", "c", "code.h"], check=True)

    print("--- Running `make all`")
    subprocess.run(["make", "all"], check=True)

    print("--- Running `bin/lbvm`")
    if dbg:
        subprocess.run(["bin/lbvm", "--dbg"], check=True)
    else:
        subprocess.run(["bin/lbvm"], check=True)


if __name__ == "__main__":
    main()
