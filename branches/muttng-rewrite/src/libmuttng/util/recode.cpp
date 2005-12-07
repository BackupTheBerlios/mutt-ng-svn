/** @ingroup libmuttng_util */
/**
 * @file libmuttng/util/recode.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Encoding/Decoding routines
 */
#include "libmuttng/util/recode.h"

void recode_encode_qp (buffer_t* buf) { (void) buf; }
int recode_decode_qp (buffer_t* buf) { (void) buf; return 0; }

void recode_encode_base64 (buffer_t* buf) { (void) buf; }
int recode_decode_base64 (buffer_t* buf) { (void) buf; return 0; }
