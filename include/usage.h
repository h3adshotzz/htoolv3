//===----------------------------------------------------------------------===//
//
//                         === The HTool Project ===
//
//  This  document  is the property of "Is This On?" It is considered to be
//  confidential and proprietary and may not be, in any form, reproduced or
//  transmitted, in whole or in part, without express permission of Is This
//  On?.
//
//  Copyright (C) 2022, Harry Moulton - Is This On? Holdings Ltd
//
//  Harry Moulton <me@h3adsh0tzz.com>
//
//===----------------------------------------------------------------------===//

#ifndef __HTOOL_USAGE_H__
#define __HTOOL_USAGE_H__


void general_usage (int argc, char *argv[], int err);
void file_subcommand_usage (int argc, char *argv[], int err);
void macho_subcommand_usage (int argc, char *argv[], int err);
void analyse_subcommand_usage (int argc, char *argv[], int err);
void disass_subcommand_usage (int argc, char *argv[], int err);

#endif /* __htool_usage_h__ */
