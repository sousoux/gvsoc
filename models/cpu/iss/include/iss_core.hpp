/*
 * Copyright (C) 2018 ETH Zurich and University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * Authors: Germain Haugou, ETH (germain.haugou@iis.ee.ethz.ch)
 */

#ifndef __CPU_ISS_ISS_CORE_HPP
#define __CPU_ISS_ISS_CORE_HPP

#include "types.hpp"

static inline void iss_exec_insn_stall(iss *iss);
static inline void iss_exec_insn_resume(iss *iss);

#include "utils.hpp"
#include "platform_wrapper.hpp"
#include "regs.hpp"
#include "perf.hpp"
#include "lsu.hpp"
#include "prefetcher.hpp"
#include "insn_cache.hpp"
#include "irq.hpp"
#include "exceptions.hpp"
#include "exec.hpp"


int iss_open(iss_t *iss);
void iss_start(iss_t *iss);


void iss_pc_set(iss_t *iss, iss_addr_t value);

iss_insn_t *iss_decode_pc(iss_t *cpu, iss_insn_t *pc);
iss_insn_t *iss_decode_pc_noexec(iss_t *cpu, iss_insn_t *pc);
void iss_decode_activate_isa(iss_t *cpu, char *isa);





void iss_csr_init(iss *iss);
bool iss_csr_read(iss *iss, iss_reg_t reg, iss_reg_t *value);
bool iss_csr_write(iss *iss, iss_reg_t reg, iss_reg_t value);


#endif
