# Usage:
#
# python ./test.py TEST_NAME



from sys import argv
from os import system
from typing import Dict, List

TEST_RES_FILE = "test.res"


def parse_test(STEP: str) -> Dict[str, str]:
    ret = {}
    status = 0
    with open("../mal/tests/" + STEP) as test:
        lines = test.readlines()
        key = ""
        value = ""
        for line in lines:
            if line.startswith(";;"):
                continue
            if line.startswith(";=>") and status == 1:
                value = line[3:]
                ret[key] = value.strip()
                status = 0
                continue
            else:
                status = 1
                key = (
                    line.replace("\n", "")
                    .replace("\\", "\\\\")
                    .replace('"', '\\"')
                    .replace("'", "'")
                    .replace("`", "\\`")
                )
                continue

    if status == 0:
        pass
    else:
        print("error while parsing", STEP)
    return ret


def do_test(tests: Dict[str, str]) -> List[str]:
    for t in tests.keys():
        system('build/test "' + t + '"' + " >> " + TEST_RES_FILE)
    res = []
    with open(TEST_RES_FILE) as results:
        res = results.readlines()
    for i, l in enumerate(res):
        res[i] = l[:len(l)-2]
    return res


def check_test(tests: Dict[str, str], res: List[str]) -> List[bool]:
    ret = []
    for i, (_, k) in enumerate(tests.items()):
        ret.append(k == res[i])
    return ret


if __name__ == "__main__":
    if len(argv) > 1:
        STEP = argv[1]
        print("testing ", STEP)
        print("-----------------\n")
        tests = parse_test(STEP)
        res = do_test(tests)
        for i, el in enumerate(check_test(tests, res)):
            print(f"{i}: {el}")
