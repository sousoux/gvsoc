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
#include <string.h>
#include "vp/itf/wire.hpp"

#include "archi/timer/timer_v2.h"

class timer : public vp::component
{

public:

  timer(const char *config);

  int build();
  void start();

  static vp::io_req_status_e req(void *__this, vp::io_req *req);

private:

  vp::trace     trace;
  vp::io_slave in;

  void sync();
  void reset();
  void depack_config(int counter, uint32_t configuration);
  void timer_reset(int counter);
  vp::io_req_status_e handle_configure(int counter, uint32_t *data, unsigned int size, bool is_write);
  vp::io_req_status_e handle_value(int counter, uint32_t *data, unsigned int size, bool is_write);
  vp::io_req_status_e handle_compare(int counter, uint32_t *data, unsigned int size, bool is_write);
  void check_state();
  uint64_t get_remaining_cycles(bool is_64, int counter);
  void check_state_counter(bool is_64, int counter);
  static void event_handler(void *__this, vp::clock_event *event);
  uint64_t get_compare_value(bool is_64, int counter);
  uint64_t get_value(bool is_64, int counter);
  void set_value(bool is_64, int counter, uint64_t new_value);

  vp::wire_master<bool> irq_itf[2];

  uint32_t value[2];
  uint32_t config[2];
  uint32_t compare_value[2];

  bool is_enabled[2];
  bool irq_enabled[2];
  bool iem[2];
  bool cmp_clr[2];
  bool one_shot[2];
  bool prescaler[2];
  bool ref_clock[2];
  uint8_t prescaler_value[2];

  bool is_64;

  int64_t sync_time;

  vp::clock_event *event;
};

timer::timer(const char *config)
: vp::component(config)
{

}

void timer::sync()
{
  int64_t cycles = get_cycles() - sync_time;
  sync_time = get_cycles();

  if (is_64 && is_enabled[0])
  {
    *(int64_t *)value += cycles;
  }
  else
  {
    if (is_enabled[0]) value[0] += cycles;
    if (is_enabled[1]) value[1] += cycles;
  }
}

uint64_t timer::get_remaining_cycles(bool is_64, int counter)
{
  uint64_t cycles;

  if (is_64) {
    // No need to check overflow on 64 bits the engine is anyway having 64 bits timestamps
    cycles = *(uint64_t *)compare_value - *(uint64_t *)value;
  } else {
    cycles = (uint32_t)(compare_value[counter] - value[counter]);
    if (cycles == 0) cycles = 0x100000000;
  }
  if (prescaler[counter]) return cycles * (prescaler_value[counter] + 1);
  else return cycles;
}

uint64_t timer::get_compare_value(bool is_64, int counter)
{
  if (is_64) return *(uint64_t *)compare_value;
  else return compare_value[counter];
}

uint64_t timer::get_value(bool is_64, int counter)
{
  if (is_64) return *(uint64_t *)value;
  else return value[counter];
}

void timer::set_value(bool is_64, int counter, uint64_t new_value)
{
  if (is_64) *(uint64_t *)value = new_value;
  else value[counter] = new_value;
}

void timer::check_state_counter(bool is_64, int counter)
{
  if (is_enabled[counter] && get_compare_value(is_64, counter) == get_value(is_64, counter))
  {
    trace.msg("Reached compare value (timer: %d)\n", counter);

    if (cmp_clr[counter]) {
      trace.msg("Clearing timer due to compare value (timer: %d)\n", counter);
      set_value(is_64, counter, 0);
    }

    if (irq_enabled[counter])
    {
      trace.msg("Raising interrupt (timer: %d)\n", counter);
      if (!irq_itf[counter].is_bound())
        trace.warning("Trying to send timer interrupt while irq port is not connected (irq: %d)\n", counter);
      else
        irq_itf[counter].sync(true);
    }

    if (one_shot[counter]) {
      trace.msg("Reached one-shot end (timer: %d)\n", counter);
      is_enabled[counter] = false;
    }

  }

  if (is_enabled[counter] && !ref_clock[counter] && (irq_enabled[counter] || cmp_clr[counter]))
  {
    int64_t cycles = get_remaining_cycles(is_64, counter);

    if (cycles != 0) {
      trace.msg("Timer is enabled, reenqueueing event (timer: %d, diffCycles: 0x%lx)\n", counter, cycles);
      event_reenqueue(event, cycles);
    }

  }
}

void timer::event_handler(void *__this, vp::clock_event *event)
{
  timer *_this = (timer *)__this;
  _this->sync();
  _this->check_state();
}

void timer::check_state()
{
  if (is_64)
  {
    check_state_counter(true, 0);
  }
  else
  {
    check_state_counter(false, 0);
    check_state_counter(false, 1);
  }
}

void timer::timer_reset(int counter)
{
  trace.msg("Resetting timer (timer: %d)\n", counter);

  if (is_64) *(int64_t *)value = 0;
  else value[counter] = 0;
}

vp::io_req_status_e timer::handle_configure(int counter, uint32_t *data, unsigned int size, bool is_write)
{
  if (is_write)
  {
    config[counter] = *data;
    depack_config(counter, config[counter]);

    trace.msg("Modified configuration (timer: %d, enabled: %d, irq: %d, iem: %d, cmp-clr: %d, one-shot: %d, prescaler: %d, prescaler value: 0x%x, is64: %d)\n", 
      counter, is_enabled[counter], irq_enabled[counter], iem[counter], cmp_clr[counter], one_shot[counter], prescaler[counter], prescaler_value[counter], is_64);

    if ((config[counter] >> PLP_TIMER_RESET_BIT) & 1) timer_reset(counter);

    // Put back reserved bits to 0 in case they were written
    uint32_t setMask = (1 << PLP_TIMER_ENABLE_BIT) | (1 << PLP_TIMER_IRQ_ENABLE_BIT) | (1 << PLP_TIMER_IEM_BIT) | (1 << PLP_TIMER_CMP_CLR_BIT) | (1 << PLP_TIMER_ONE_SHOT_BIT) | (1 << PLP_TIMER_PRESCALER_ENABLE_BIT) | (((1 << PLP_TIMER_PRESCALER_VALUE_BITS)-1)<<PLP_TIMER_PRESCALER_VALUE_BITS);
    if (counter == 0) setMask |= 1 << PLP_TIMER_64_BIT;

    config[counter] &= setMask;

    check_state();
  }
  else
  {
    *data = (config[counter] & ~(1<<PLP_TIMER_ENABLE_BIT)) | (is_enabled[counter] << PLP_TIMER_ENABLE_BIT);
  }

  return vp::IO_REQ_OK;
}

vp::io_req_status_e timer::handle_value(int counter, uint32_t *data, unsigned int size, bool is_write)
{
  if (is_write)
  {
    trace.msg("Modified value (timer: %d, value: 0x%x)\n", *data);
    value[counter] = *data;
    check_state();
  }
  else
    *data = value[counter];

  return vp::IO_REQ_OK;

}

vp::io_req_status_e timer::handle_compare(int counter, uint32_t *data, unsigned int size, bool is_write)
{
  if (is_write)
  {
    trace.msg("Modified compare value (timer: %d, value: 0x%x)\n", *data);
    compare_value[counter] = *data;
    check_state();
  }
  else
  {
    *data = compare_value[counter];
  }
  return vp::IO_REQ_OK;

}

void timer::depack_config(int counter, uint32_t configuration)
{
  is_enabled[counter] = (configuration >> PLP_TIMER_ENABLE_BIT) & 1;
  irq_enabled[counter] = (configuration >> PLP_TIMER_IRQ_ENABLE_BIT) & 1;
  iem[counter] = (configuration >> PLP_TIMER_IEM_BIT) & 1;
  cmp_clr[counter] = (configuration >> PLP_TIMER_CMP_CLR_BIT) & 1;
  one_shot[counter] = (configuration >> PLP_TIMER_ONE_SHOT_BIT) & 1;
  prescaler[counter] = (configuration >> PLP_TIMER_PRESCALER_ENABLE_BIT) & 1;
  ref_clock[counter] = (configuration >> PLP_TIMER_CLOCK_SOURCE_BIT) & 1;
  prescaler_value[counter] = (configuration >> PLP_TIMER_PRESCALER_VALUE_BIT) & ((1<<PLP_TIMER_PRESCALER_VALUE_BITS)-1);
  if (counter == 0) is_64 = (configuration >> PLP_TIMER_64_BIT) & 1;
}

vp::io_req_status_e timer::req(void *__this, vp::io_req *req)
{
  timer *_this = (timer *)__this;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();
  bool is_write = req->get_is_write();

  _this->trace.msg("Timer access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, is_write);

  if (size > 4) return vp::IO_REQ_INVALID;

  // As the timer model is not updating counters at each cycle, we need to synchronize
  // them anytime we want to use them
  _this->sync();

  switch (offset) {

    case PLP_TIMER_CFG_REG_HI:
      return  _this->handle_configure(1, (uint32_t *)data, size, is_write);

    case PLP_TIMER_CFG_REG_LO:
      return  _this->handle_configure(0, (uint32_t *)data, size, is_write);

    case PLP_TIMER_VALUE_HI:
      return  _this->handle_value(1, (uint32_t *)data, size, is_write);

    case PLP_TIMER_VALUE_LO:
      return  _this->handle_value(0, (uint32_t *)data, size, is_write);

    case PLP_TIMER_CMP_HI:
      return  _this->handle_compare(1, (uint32_t *)data, size, is_write);

    case PLP_TIMER_CMP_LO:
      return  _this->handle_compare(0, (uint32_t *)data, size, is_write);

    case PLP_TIMER_RESET_LOW: {
      uint32_t configuration = _this->config[0] | (1<<PLP_TIMER_RESET_BIT);
      return  _this->handle_configure(0, (uint32_t *)&configuration, 4, true);
    }

    case PLP_TIMER_RESET_HIGH: {
      uint32_t configuration = _this->config[1] | (1<<PLP_TIMER_RESET_BIT);
      return  _this->handle_configure(1, (uint32_t *)&configuration, 4, true);
    }

    case PLP_TIMER_START_LOW: {
      uint32_t configuration = _this->config[0] | (1<<PLP_TIMER_ENABLE_BIT);
      return  _this->handle_configure(0, (uint32_t *)&configuration, 4, true);
    }

    case PLP_TIMER_START_HIGH: {
      uint32_t configuration = _this->config[1] | (1<<PLP_TIMER_ENABLE_BIT);
      return  _this->handle_configure(1, (uint32_t *)&configuration, 4, true);
    }
  }

  return vp::IO_REQ_OK;
}


int timer::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);

  in.set_req_meth(&timer::req);
  new_slave_port("input", &in);

  event = event_new(timer::event_handler);

  new_master_port("irq_itf_0", &irq_itf[0]);
  new_master_port("irq_itf_1", &irq_itf[1]);

  return 0;
}

void timer::reset()
{
  for (int i=0; i<2; i++)
  {
    value[i] = 0;
    config[i] = 0;
    compare_value[i] = 0;
    depack_config(i, config[i]);
  }

  sync_time = get_cycles();
}

void timer::start()
{
  reset();
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new timer(config);
}
