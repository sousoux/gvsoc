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

#include <vp/vp.hpp>
#include <vp/itf/io.hpp>
#include <stdio.h>
#include <vp/itf/wire.hpp>
#include <string.h>

class pmu : public vp::component
{

public:

  pmu(const char *config);

  int build();
  void start();

  static vp::io_req_status_e req(void *__this, vp::io_req *req);

private:

  void reset();

  vp::trace     trace;
  vp::io_slave in;

};

pmu::pmu(const char *config)
: vp::component(config)
{

}

vp::io_req_status_e pmu::req(void *__this, vp::io_req *req)
{
  pmu *_this = (pmu *)__this;

  uint64_t offset = req->get_addr();
  //uint32_t *data = (uint32_t *)req->get_data();
  uint64_t size = req->get_size();
  bool is_write = req->get_is_write();

  _this->trace.msg("IO access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, is_write);

  return vp::IO_REQ_OK;
}


int pmu::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);
  in.set_req_meth(&pmu::req);
  new_slave_port("input", &in);

  return 0;
}

void pmu::reset()
{
}

void pmu::start()
{
  reset();
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new pmu(config);
}
