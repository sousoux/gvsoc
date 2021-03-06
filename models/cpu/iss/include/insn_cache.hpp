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

#ifndef __CPU_ISS_ISS_INSN_CACHE_HPP
#define __CPU_ISS_ISS_INSN_CACHE_HPP

int insn_cache_init(iss *iss);
void iss_cache_flush(iss *iss);
iss_insn_t *insn_cache_get(iss *iss, iss_addr_t pc);
iss_insn_t *insn_cache_get_decoded(iss *iss, iss_addr_t pc);

#endif
