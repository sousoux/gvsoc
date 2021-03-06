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

#ifndef __CPU_ISS_PLATFORM_WRAPPER_HPP
#define __CPU_ISS_PLATFORM_WRAPPER_HPP

#include "types.hpp"
#include "iss_wrapper.hpp"
#include <string.h>

static inline int iss_io_req(iss_t *_this, uint64_t addr, uint8_t *data, uint64_t size, bool is_write)
{
  return _this->data.req(&_this->io_req);
}

static inline int iss_fetch_req(iss_t *_this, uint64_t addr, uint8_t *data, uint64_t size, bool is_write)
{
  vp::io_req *req = &_this->fetch_req;
  req->init();
  req->set_addr(addr);
  req->set_size(size);
  req->set_is_write(is_write);
  req->set_data(data);
  return _this->fetch.req(req);
}

static inline int iss_irq_ack(iss_t *iss, int irq)
{
  iss->decode_trace.msg("Acknowledging interrupt (irq: %d)\n", irq);
  iss->irq_ack_itf.sync(irq);
}

#endif
