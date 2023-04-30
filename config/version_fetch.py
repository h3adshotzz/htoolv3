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

import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--file", action="store", help="MasterVersion file")
    args = parser.parse_args()

    with open(args.file, "r") as f:
        data = f.readlines()
    
    build_version = data[0].strip().split(".")
    build_version_major, build_version_minor, build_version_revision = map(int, build_version)
    source_version_major, source_version_minor = map(int, data[1:3])

    SOURCE_VERSION = f"htool-{build_version_major}{build_version_minor}{build_version_revision}.{source_version_major}.{source_version_minor}"
    print("{}".format(SOURCE_VERSION))
