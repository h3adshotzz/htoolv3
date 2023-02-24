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
import yaml
import argparse

from dataclasses import dataclass
from pathlib import Path

from common import HToolTestCase, HToolTestRun, HToolTestSuite, run_test_case
from macho_test_parser import TEST_SUITE_NAME_MACHO, run_macho_tests


# Clone the repository and checkout the correct branch
#
def init_tests_repository(remote, branch):
    
    # Check if the repository exists in the current path
    path = Path.cwd() / Path("htoolv3-tests")
    if path.exists():
        return path
    
    # Try cloning the provided remote and branch
    if os.system("git clone {} -b {}".format(remote, branch)) == 0:
        print("[*] Clone tests repository")
    else:
        print("[*] ERROR: Failed to clone tests repository")
        sys.exit(1)

    # The path should now be valid
    return path


# Load YAML config file
#
def load_config_yaml(config):

    config_path = Path(config)
    if config_path.exists():
        with open(config_path, "r") as stream:
            try: yml_data = yaml.safe_load(stream)
            except yaml.YAMLError as exc:
                print("[*] ERROR: Failed to load config file: {}".format(config_path))
                sys.exit(1)
    else:
        print("[*] ERROR: Config path is not valid: {}".format(config_path))
        sys.exit(1)

    return yml_data


# Parse the loaded config file into a TestSuite type.
#
def parse_config_yaml(config):
    suite_data = config["TestSuite"]

    tests = []
    for s in suite_data["tests"]:
        test = HToolTestCase(s["name"], s["flags"], s["file"])
        tests.append(test)

    suite = HToolTestSuite(suite_data["suite_name"], suite_data["git"]["remote"], suite_data["git"]["branch"], tests)
    return suite


###############################################################################

if __name__ == "__main__":

    # Setup parser
    parser = argparse.ArgumentParser(description="HTool Test Runner")
    parser.add_argument("-c", "--config", action="store", help="Config file containing tests to run.", required=True)
    parser.add_argument("-b", "--bin", action="store", help="HTool binary to use to run tests.", required=True)

    # Parse args
    args = parser.parse_args()
    print("config file: {}\nhtool bin: {}\n".format(args.config, args.bin))

    data = load_config_yaml(args.config)
    suite = parse_config_yaml(data)

    if suite.name == TEST_SUITE_NAME_MACHO:
        run_macho_tests(suite, args.bin)
