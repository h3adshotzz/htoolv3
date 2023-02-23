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
    res: int
    logs: list[str]
    errors: list[str]
    warnings: list[str]
    liberrors: list[str]

def run_test_case(test_case, bin):
    f = os.popen("{} {} {}".format(bin, test_case.flags, test_case.file))
    data = f.read().split("\n")
    status = f.close()

    logs = []
    errors = []
    warnings = []
    liberrors = []
    for l in data:
        if "[TEST_CI]" in l: logs.append(l)
        elif "[*Error*]" in l: errors.append(l)
        elif "[Error]" in l: liberrors.append(l)
        elif "[Warning]" in l: warnings.append(l)

    return HToolTestRun(res=status, logs=logs, errors=errors, warnings=warnings, liberrors=liberrors)

def find_log(data, log):
    for l in data:
        if log in l:
            return True
    return False

def parse_test_case(suite_name, case_name, log_string, failure_msg, log_data):
    case = TestCase("{}.{}".format(suite_name, case_name))
    if not find_log(log_data, log_string):
        case.add_failure_info(failure_msg)
    return case

def parse_htool_errors(suite_name, case_name, error_data):
    case = TestCase("{}.{}".format(suite_name, case_name))
    if len(error_data):
        res = "{} HTool errors encountered: ".format(len(error_data))

        count = 0
        for e in error_data:
            e = e.split("[*Error*]")[1][1:]
            res += "\nErr{}: {}".format(count, e)

        case.add_failure_info(res)
    return case

def parse_libhelper_errors(suite_name, case_name, error_data):
    case = TestCase("{}.{}".format(suite_name, case_name))
    if len(error_data):
        res = "{} Libhelper errors encountered: ".format(len(error_data))

        count = 0
        for e in error_data:
            e = e.split("[Error]")[1][1:]
            res += "\nErr{}: {}".format(count, e)

        case.add_failure_info(res)
    return case

def parse_warnings(suite_name, case_name, warnings_data):
    case = TestCase("{}.{}".format(suite_name, case_name))
    if len(warnings_data):
        res = "{} warnings encountered: ".format(len(warnings_data))

        count = 0
        for e in warnings_data:
            e = e.split("[Warning]")[1][1:]
            res += "\nwarning:{}: {}".format(count, e)

        case.add_skipped_info(res)
    return case