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
#include <vp/itf/clk.hpp>
#include <vp/clock/clock_engine.hpp>

class clock_domain : public vp::clock_engine
{

public:

  clock_domain(const char *config);

  int build();

  void pre_start();

private:

  vp::clk_master out;


};


clock_domain::clock_domain(const char *config)
: vp::clock_engine(config)
{

  freq = get_config_int("frequency");

  period = 1e12 / freq;
  
}

int clock_domain::build()
{
  new_master_port("out", &out);
  return 0;
}

void clock_domain::pre_start()
{
  out.reg(this);
}


vp::clock_engine::clock_engine(const char *config)
  : vp::time_engine_client(config), cycles(0)
{
  delayed_queue = NULL;
  for (int i=0; i<CLOCK_EVENT_QUEUE_SIZE; i++)
  {
    event_queue[i] = NULL;
  }
  current_cycle = 0;
}


extern "C" void vp_set_time_engine(void *comp, void *engine)
{
  ((vp::clock_engine *)comp)->set_time_engine((vp::time_engine *)engine);
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new clock_domain(config);
}
