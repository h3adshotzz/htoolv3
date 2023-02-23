##===----------------------------------------------------------------------===//
##
##                         === The HTool Project ===
##
##  This  document  is the property of "Is This On?" It is considered to be
##  confidential and proprietary and may not be, in any form, reproduced or
##  transmitted, in whole or in part, without express permission of Is This
##  On?.
##
##  Copyright (C) 2023, Is This On? Holdings Limited
##
##  Harry Moulton <me@h3adsh0tzz.com>
##
##===----------------------------------------------------------------------===//

import os
from dataclasses import dataclass
from junit_xml import TestSuite, TestCase

@dataclass
class HToolTestCase:
    name: str
    flags: str
    file: str

@dataclass
class HToolTestSuite:
    name: str
    gitremote: str
    gitbranch: str
    tests: list[HToolTestCase]

@dataclass
class ParsedLogs:
    warnings: str
    errors: str

@dataclass
class HToolTestRun:
    #name: str
    output: list[str]
    res: int
    #messages: ParsedLogs

def run_test_case(test_case, bin):
    f = os.popen("{} {} {}".format(bin, test_case.flags, test_case.file))
    data = f.read().split("\n")
    status = f.close()

    return HToolTestRun(res=status, output=data)

def parse_message_logs(name, output):
    tc = TestCase(name)

    for a in output:
        if "[*Error*]" in a:
            print(a)
            tc.add_failure_info(a.split("[*Error*]")[1][1:])

    print(vars(tc))
    return tc