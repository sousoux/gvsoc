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
#include <vp/itf/wire.hpp>
#include <stdio.h>
#include <string.h>
#include "archi/itc/itc_v1.h"

class itc : public vp::component
{

public:

  itc(const char *config);

  void build();
  void start();

  static vp::io_req_status_e req(void *__this, vp::io_req *req);

  vp::io_req_status_e itc_mask_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write);
  vp::io_req_status_e itc_mask_set_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write);
  vp::io_req_status_e itc_mask_clr_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write);
  vp::io_req_status_e itc_status_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write);
  vp::io_req_status_e itc_status_set_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write);
  vp::io_req_status_e itc_status_clr_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write);
  vp::io_req_status_e itc_ack_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write);
  vp::io_req_status_e itc_ack_set_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write);
  vp::io_req_status_e itc_ack_clr_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write);
  vp::io_req_status_e itc_fifo_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write);

  void itc_status_setValue(uint32_t value);

  void check_state();

  void reset();

  static void irq_ack_sync(void *__this, int irq);

private:

  vp::trace     trace;
  vp::io_slave in;

  uint32_t ack;
  uint32_t status;
  uint32_t mask;

  int nb_free_events;
  int fifo_event_head;
  int fifo_event_tail;
  int *fifo_event;

  int nb_fifo_events;
  int fifo_irq;

  int sync_irq;

  vp::wire_master<int>    irq_req_itf;
  vp::wire_slave<int>     irq_ack_itf;

};

itc::itc(const char *config)
: vp::component(config)
{

}

void itc::itc_status_setValue(uint32_t value)
{
  trace.msg("Updated irq status (value: 0x%x)\n", value);
  status = value;
  check_state();
}

void itc::check_state()
{
  uint32_t status_masked = status & mask;
  int irq = status_masked ? 31 - __builtin_clz(status_masked) : -1;

  if (irq != sync_irq) {
    trace.msg("Updating irq req (irq: %d)\n", irq);
    sync_irq = irq;
    irq_req_itf.sync(irq);
  }

}

vp::io_req_status_e itc::itc_mask_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write)
{
  if (!is_write) *data = mask;
  else {
    mask = *data;
    trace.msg("Updated irq mask (value: 0x%x)\n", mask);
  }

  check_state();

  return vp::IO_REQ_OK;
}

vp::io_req_status_e itc::itc_mask_set_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write)
{
  if (!is_write) return vp::IO_REQ_INVALID;

  mask |= *data;
  trace.msg("Updated irq mask (value: 0x%x)\n", mask);

  check_state();
  
  return vp::IO_REQ_OK;
}

vp::io_req_status_e itc::itc_mask_clr_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write)
{
  if (!is_write) return vp::IO_REQ_INVALID;

  mask &= ~(*data);
  trace.msg("Updated irq mask (value: 0x%x)\n", mask);

  check_state();
  
  return vp::IO_REQ_OK;
}

vp::io_req_status_e itc::itc_status_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write)
{
  if (!is_write) *data = status;
  else {
    itc_status_setValue(*data);
  }

  return vp::IO_REQ_OK;
}

vp::io_req_status_e itc::itc_status_set_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write)
{
  if (!is_write) return vp::IO_REQ_INVALID;

  itc_status_setValue(status | *data);

  return vp::IO_REQ_OK;
}

vp::io_req_status_e itc::itc_status_clr_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write)
{
  if (!is_write) return vp::IO_REQ_INVALID;

  itc_status_setValue(status & ~(*data));

  return vp::IO_REQ_OK;
}

vp::io_req_status_e itc::itc_ack_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write)
{
  if (!is_write) *data = ack;
  else {
    ack = *data;
    trace.msg("Updated irq ack (value: 0x%x)\n", ack);
  }

  return vp::IO_REQ_OK;
}

vp::io_req_status_e itc::itc_ack_set_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write)
{
  if (!is_write) return vp::IO_REQ_INVALID;

  ack |= *data;
  trace.msg("Updated irq ack (value: 0x%x)\n", ack);

  return vp::IO_REQ_OK;
}

vp::io_req_status_e itc::itc_ack_clr_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write)
{
  if (!is_write) return vp::IO_REQ_INVALID;

  ack &= ~(*data);
  trace.msg("Updated irq ack (value: 0x%x)\n", ack);

  return vp::IO_REQ_OK;
}

vp::io_req_status_e itc::itc_fifo_ioReq(uint32_t offset, uint32_t *data, uint32_t size, bool is_write)
{
  if (is_write) return vp::IO_REQ_INVALID;

  if (nb_free_events == nb_fifo_events) {
    trace.warning("Reading FIFO with no event\n");
    *data = 0;
  } else {
    trace.msg("Popping event from FIFO (id: %d)\n", fifo_event[fifo_event_tail]);
    *data = fifo_event[fifo_event_tail];
    fifo_event_tail++;
    if (fifo_event_tail == nb_fifo_events) fifo_event_tail = 0;
    nb_free_events++;
    if (nb_free_events != nb_fifo_events) {
      itc_status_setValue(status | (1<<fifo_irq));
    }
  }

  return vp::IO_REQ_OK;
}

vp::io_req_status_e itc::req(void *__this, vp::io_req *req)
{
  itc *_this = (itc *)__this;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();
  bool is_write = req->get_is_write();

  _this->trace.msg("Itc access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, is_write);

  if (size != 4) return vp::IO_REQ_INVALID;

  switch (offset) {
    case ARCHI_ITC_MASK:       return _this->itc_mask_ioReq       (offset, (uint32_t *)data, size, is_write);
    case ARCHI_ITC_MASK_SET:   return _this->itc_mask_set_ioReq   (offset, (uint32_t *)data, size, is_write);
    case ARCHI_ITC_MASK_CLR:   return _this->itc_mask_clr_ioReq   (offset, (uint32_t *)data, size, is_write);
    case ARCHI_ITC_STATUS:     return _this->itc_status_ioReq     (offset, (uint32_t *)data, size, is_write);
    case ARCHI_ITC_STATUS_SET: return _this->itc_status_set_ioReq (offset, (uint32_t *)data, size, is_write);
    case ARCHI_ITC_STATUS_CLR: return _this->itc_status_clr_ioReq (offset, (uint32_t *)data, size, is_write);
    case ARCHI_ITC_ACK:        return _this->itc_ack_ioReq        (offset, (uint32_t *)data, size, is_write);
    case ARCHI_ITC_ACK_SET:    return _this->itc_ack_set_ioReq    (offset, (uint32_t *)data, size, is_write);
    case ARCHI_ITC_ACK_CLR:    return _this->itc_ack_clr_ioReq    (offset, (uint32_t *)data, size, is_write);
    case ARCHI_ITC_FIFO:       return _this->itc_fifo_ioReq       (offset, (uint32_t *)data, size, is_write);
  }

  return vp::IO_REQ_OK;
}

void itc::irq_ack_sync(void *__this, int irq)
{
  itc *_this = (itc *)__this;

  _this->trace.msg("Received IRQ acknowledgement (irq: %d)\n", irq);

  _this->itc_status_setValue(_this->status & ~(1<<irq));
  _this->ack |= 1<<irq;

  _this->trace.msg("Updated irq ack (value: 0x%x)\n", _this->ack);
}

void itc::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);

  in.set_req_meth(&itc::req);
  new_slave_port("in", &in);

  new_master_port("irq_req", &irq_req_itf);

  irq_ack_itf.set_sync_meth(&itc::irq_ack_sync);
  new_slave_port("irq_ack", &irq_ack_itf);

  nb_fifo_events = get_config_int("**/nb_fifo_events");
  fifo_irq = get_config_int("**/fifo_irq");

  fifo_event = new int[nb_fifo_events];

}

void itc::start()
{
  reset();
}


void itc::reset()
{
  status = 0;
  mask   = 0;
  ack    = 0;

  sync_irq = -1;

  nb_free_events = nb_fifo_events;
  fifo_event_head = 0;
  fifo_event_tail = 0;
}



extern "C" void *vp_constructor(const char *config)
{
  return (void *)new itc(config);
}