# Usage:
#
# python ./test.py TEST_NAME


from sys import argv
from os.path import abspath
from subprocess import run
from typing import Dict, List, Tuple
import difflib

TEST_RES_FILE = "test.res"


def parse_test(STEP: str) -> List[Tuple[str, str]]:
    """
    Returns a list of pair (INPUT, EXPECTED_OUTPUT)
    """
    ret: List[Tuple[str, str]] = []
    status = 0
    with open(STEP) as test:
        lines = test.readlines()
        key = ""
        value = ""
        for line in lines:
            if line.startswith(";;") or line.startswith(";>>>") or len(line) == 0:
                continue
            if line.startswith(";=>") and status == 1:
                value = line[3:]
                ret.append((key.strip(), value.strip()))
                status = 0
                continue
            else:
                status = 1
                key = line.strip() # (
                 #    line.replace('"', '"').strip()
                 # )
                continue
    if status == 0:
        pass
    else:
        print("error while parsing", STEP)
    return ret


def do_test(tests: List[Tuple[str, str]]) -> List[str]:
    """
    Returns a list of string ACTUAL_OUTPUT
    """
    res: List[str] = []
    exec_path = abspath("./build/test.exe")
    for i, t in enumerate(tests):
        ret = run(
            args=[exec_path, t[0]],
            shell=True,
            capture_output=True,
        )
        stdout = ret.stdout.decode("utf-8")
        # output = stdout[len(stdout)-1] if len(stdout) > 0 else str(stdout)
       
        # print('------------------')
        # print(stdout)
        # print('------------------')
        res.append(stdout)
    return res


def check_test(tests: List[Tuple[str, str]], res: List[str]) -> List[bool]:
    ret = []
    for i, t in enumerate(tests):
        result = res[i]
        ret.append(t[1].strip() == result.strip())
    return ret


if __name__ == "__main__":
    if len(argv) > 1:
        STEP = argv[1]
        tests = parse_test(STEP)
        res = do_test(tests)

        print("testing ", STEP)
        print("-----------------\n")
        print("RESULTS:")

        
        for i, el in enumerate(check_test(tests, res)):
            print(
                f"""
            N:         {i}
            INPUT:     {tests[i][0]}
            EXPECTED:  {tests[i][1]}
            OUTPUT:    {res[i]}
            RESULT:    {el}
            -----------------"""
            )
