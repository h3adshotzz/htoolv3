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
import sys

from common import run_test_case, parse_test_case, parse_htool_errors, parse_libhelper_errors, parse_warnings, find_log
from junit_xml import TestSuite, TestCase

TEST_SUITE_NAME_MACHO = "macho-tests"

def RunTest_valid_macho_file(test, bin):
    SUITE_NAME = "Mach-O Tests"

    # This returns an HToolTestRun object, with a logs array that only contains
    # logs starting with the string [TEST CI], meaning the entire htool output
    # isn't constantly being iterated.
    results = run_test_case(test, bin)
    cases = []

    print(results.logs)

    # File loaded correctly
    CASE_NAME = "file_loaded_correctly"
    LOG_STRING = "File successfully loaded"
    FAIL_STRING = "Failed to load file"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results.logs))

    # Mach Header
    CASE_NAME = "mach_header_printed"
    LOG_STRING = "Printed Mach-O Header"
    FAIL_STRING = "Failed to print Mach-O Header"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results.logs))

    # Segment commands
    CASE_NAME = "mach_segment_commands_printed"
    LOG_STRING = "Printed Mach-O Segment Commands"
    FAIL_STRING = "Failed to print Mach-O Segment Commands"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results.logs))

    # Load commands
    CASE_NAME = "mach_load_commands_printed"
    LOG_STRING = "Printed Mach-O Load Commands"
    FAIL_STRING = "Failed to print Mach-O Load Commands"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results.logs))

    # Check for any errors
    cases.append(parse_htool_errors(SUITE_NAME, "HTool Errors", results.errors))
    cases.append(parse_libhelper_errors(SUITE_NAME, "Libhelper Errors", results.liberrors))
    cases.append(parse_warnings(SUITE_NAME, "Warnings", results.warnings))

    # Check for any segfaults


    return cases


def run_macho_tests(test_suite, bin):

    suites = []

    for test in test_suite.tests:
        if test.name == "macho-test_valid-macho-file":
            
            print("macho-test_valid-macho-file")
            cases = RunTest_valid_macho_file(test, bin)
            suites.append(TestSuite("Mach-O Tests", cases))

        elif test.name == "macho-test_valid-fat-no-arch":
            print("macho-test_valid-fat-no-arch")
        elif test.name == "macho-test_valid-fat-with-arch":
            print("macho-test_valid-fat-with-arch")
        elif test.name == "macho-test_invalid-macho":
            print("macho-test_invalid-macho")
        elif test.name == "macho-test_no-file":
            print("macho-test_no-file")
        else:
            print("unknown test case")


    results = run_test_case(test_suite.tests[5], bin)

    #tc = parse_message_logs(test_suite.name, results.output)
    #cases = [tc]

    with open("test.xml", "w") as f:
        TestSuite.to_file(f, suites)


#    for test in test_suite.tests:
#        a = run_test_case(test, bin)
#        print("{}\n".format(a))