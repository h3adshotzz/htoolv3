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

from common import run_test_case, parse_message_logs
from junit_xml import TestSuite, TestCase

TEST_SUITE_NAME_MACHO = "macho-tests"

def run_macho_tests(test_suite, bin):

    test_cases_xml = []

    for test in test_suite.tests:
        if test.name == "macho-test_valid-macho-file":
            print("macho-test_valid-macho-file")
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

    tc = parse_message_logs(test_suite.name, results.output)
    cases = [tc]
    ts = TestSuite("suite", cases)

    with open("test.xml", "w") as f:
        TestSuite.to_file(f, [ts])


#    for test in test_suite.tests:
#        a = run_test_case(test, bin)
#        print("{}\n".format(a))