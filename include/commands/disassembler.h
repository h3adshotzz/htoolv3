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

#ifndef __HTOOL_DISASSEMBLER_H__
#define __HTOOL_DISASSEMBLER_H__

#include <stdlib.h>

#include "htool.h"
#include "htool-client.h"

typedef struct htool_disass_t
{
    htool_binary_t      *bin;
    

};

htool_return_t
htool_disassemble_binary_quick (htool_client_t *client);



#endif /* __htool_disassembler_h__ */