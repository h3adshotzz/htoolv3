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

from common import run_test_case, parse_test_case, parse_htool_errors, parse_libhelper_errors, parse_warnings, parse_runtime_errors, find_log, find_log_str
from junit_xml import TestSuite, TestCase

TEST_SUITE_NAME_MACHO = "macho-tests"

def RunTest_valid_macho_file(suite_name, test, bin):
    SUITE_NAME = suite_name

    # This returns an HToolTestRun object, with a logs array that only contains
    # logs starting with the string [TEST CI], meaning the entire htool output
    # isn't constantly being iterated.
    results = run_test_case(test, bin)
    cases = []

    # File loaded correctly
    CASE_NAME = "file_loaded_correctly"
    LOG_STRING = "File successfully loaded"
    FAIL_STRING = "Failed to load file"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results))

    # Mach Header
    CASE_NAME = "mach_header_printed"
    LOG_STRING = "Printed Mach-O Header"
    FAIL_STRING = "Failed to print Mach-O Header"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results))

    # Segment commands
    CASE_NAME = "mach_segment_commands_printed"
    LOG_STRING = "Printed Mach-O Segment Commands"
    FAIL_STRING = "Failed to print Mach-O Segment Commands"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results))

    # Load commands
    CASE_NAME = "mach_load_commands_printed"
    LOG_STRING = "Printed Mach-O Load Commands"
    FAIL_STRING = "Failed to print Mach-O Load Commands"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results))

    # Check for any errors
    cases.append(parse_htool_errors(SUITE_NAME, "HTool Errors", results))
    cases.append(parse_libhelper_errors(SUITE_NAME, "Libhelper Errors", results.liberrors))
    cases.append(parse_warnings(SUITE_NAME, "Warnings", results.warnings))

    # Check for any segfaults
    parse_runtime_errors(SUITE_NAME, cases, results)

    return cases

def RunTest_valid_fat_no_arch(suite_name, test, bin):
    SUITE_NAME = suite_name

    # This returns an HToolTestRun object, with a logs array that only contains
    # logs starting with the string [TEST CI], meaning the entire htool output
    # isn't constantly being iterated.
    results = run_test_case(test, bin)
    cases = []

    # File loaded correctly
    CASE_NAME = "file_loaded_correctly"
    LOG_STRING = "File successfully loaded"
    FAIL_STRING = "Failed to load file"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results))

    # FAT Header
    CASE_NAME = "fat_printed"
    LOG_STRING = "Printed FAT Header"
    FAIL_STRING = "Failed to print FAT Header"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results))

    # Check for any segfaults
    parse_runtime_errors(SUITE_NAME, cases, results)

    return cases

def RunTest_valid_fat_with_arch(suite_name, test, bin, arch_string):
    SUITE_NAME = suite_name

    # This returns an HToolTestRun object, with a logs array that only contains
    # logs starting with the string [TEST CI], meaning the entire htool output
    # isn't constantly being iterated.
    results = run_test_case(test, bin)
    cases = []

    # File loaded correctly
    CASE_NAME = "file_loaded_correctly"
    LOG_STRING = "File successfully loaded"
    FAIL_STRING = "Failed to load file"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results))

    # Mach Header
    CASE_NAME = "mach_header_printed"
    LOG_STRING = "Printed Mach-O Header"
    FAIL_STRING = "Failed to print Mach-O Header"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results))

    # Architecture
    case = TestCase("{}.architecture_arm64".format(SUITE_NAME))
    if not find_log(results.data, arch_string):
        case.add_failure_info(find_log_str(results.data, "CPU: "))
    cases.append(case)

    # Segment commands
    CASE_NAME = "mach_segment_commands_printed"
    LOG_STRING = "Printed Mach-O Segment Commands"
    FAIL_STRING = "Failed to print Mach-O Segment Commands"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results))

    # Load commands
    CASE_NAME = "mach_load_commands_printed"
    LOG_STRING = "Printed Mach-O Load Commands"
    FAIL_STRING = "Failed to print Mach-O Load Commands"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results))

    # Check for any errors
    cases.append(parse_htool_errors(SUITE_NAME, "HTool Errors", results))
    cases.append(parse_libhelper_errors(SUITE_NAME, "Libhelper Errors", results.liberrors))
    cases.append(parse_warnings(SUITE_NAME, "Warnings", results.warnings))

    # Check for any segfaults
    parse_runtime_errors(SUITE_NAME, cases, results)

    return cases

def RunTest_invalid_macho(suite_name, test, bin):
    SUITE_NAME = suite_name

    # This returns an HToolTestRun object, with a logs array that only contains
    # logs starting with the string [TEST CI], meaning the entire htool output
    # isn't constantly being iterated.
    results = run_test_case(test, bin)
    cases = []

    # File loaded correctly
    CASE_NAME = "file_loaded_incorrectly"
    LOG_STRING = "File could not be loaded"
    FAIL_STRING = "Failed to detect invalid file"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results))

    # Check for any segfaults
    parse_runtime_errors(SUITE_NAME, cases, results)

    return cases

def RunTest_nonexistent_file(suite_name, test, bin):
    SUITE_NAME = suite_name

    # This returns an HToolTestRun object, with a logs array that only contains
    # logs starting with the string [TEST CI], meaning the entire htool output
    # isn't constantly being iterated.
    results = run_test_case(test, bin)
    cases = []

    # File loaded correctly
    CASE_NAME = "file_does_not_exist"
    LOG_STRING = "File does not exist"
    FAIL_STRING = "Failed to detect nonexistent file"
    cases.append(parse_test_case(SUITE_NAME, CASE_NAME, LOG_STRING, FAIL_STRING, results))

    # Check for any segfaults
    parse_runtime_errors(SUITE_NAME, cases, results)

    return cases


def run_macho_tests(test_suite, bin):

    suites = []

    for test in test_suite.tests:
        if test.name == "macho-test_valid-macho-file":
            
            print("macho-test_valid-macho-file")
            SUITE_NAME = "Mach-O Tests"
            cases = RunTest_valid_macho_file(SUITE_NAME, test, bin)
            suites.append(TestSuite(SUITE_NAME, cases))

        elif test.name == "macho-test_valid-fat-no-arch":

            print("macho-test_valid-fat-no-arch")
            SUITE_NAME = "Mach-O FAT (No --arch) Tests"
            cases = RunTest_valid_fat_no_arch(SUITE_NAME, test, bin)
            suites.append(TestSuite(SUITE_NAME, cases))

        elif test.name == "macho-test_valid-fat-with-arch":

            print("macho-test_valid-fat-with-arch")
            SUITE_NAME = "Mach-O FAT (--arch=arm64e) Tests"
            cases = RunTest_valid_fat_with_arch(SUITE_NAME, test, bin, "arm64e (ARMv8.5-A, MTE)")
            suites.append(TestSuite(SUITE_NAME, cases))

        elif test.name == "macho-test_invalid-macho":

            print("macho-test_invalid-macho")
            SUITE_NAME = "Mach-O Invalid File"
            cases = RunTest_invalid_macho(SUITE_NAME, test, bin)
            suites.append(TestSuite(SUITE_NAME, cases))

        elif test.name == "macho-test_no-file":

            print("macho-test_no-file")
            SUITE_NAME = "Mach-O Nonexistent File"
            cases = RunTest_nonexistent_file(SUITE_NAME, test, bin)
            suites.append(TestSuite(SUITE_NAME, cases))


        else:
            print("unknown test case")



    with open("test.xml", "w") as f:
        TestSuite.to_file(f, suites)


#    for test in test_suite.tests:
#        a = run_test_case(test, bin)
#        print("{}\n".format(a))