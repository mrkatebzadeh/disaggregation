//#include "
#include "packet.h"
#include "params.h"
#include "factory.h"
#include "flow.h"

extern DCExpParams params;


bool FlowComparator::operator() (Flow *a, Flow *b) {
  return a->flow_priority > b->flow_priority;
//  if(a->flow_priority > b->flow_priority)
//    return true;
//  else if(a->flow_priority == b->flow_priority)
//    return a->id > b->id;
//  else
//    return false;
}

Node::Node(uint32_t id, uint32_t type) {
  this->id = id;
  this->type = type;
}

// TODO FIX superclass constructor
Host::Host(uint32_t id, double rate, uint32_t type) : Node(id, HOST)
{
  queue = Factory::get_queue(id, rate, params.queue_size, type, 0, 0);
  receiving = NULL;
}

// TODO FIX superclass constructor
Switch::Switch(uint32_t id, uint32_t switch_type) : Node(id, SWITCH)
{
  this->switch_type = switch_type;
}

CoreSwitch::CoreSwitch(uint32_t id, uint32_t nq, double rate, uint32_t type)
  : Switch(id, CORE_SWITCH)
{
  for (uint32_t i = 0; i < nq; i++) {
    queues.push_back(Factory::get_queue(i, rate, params.queue_size, type, 0, 2));
  }
}

//nq1: # host switch, nq2: # core switch
AggSwitch::AggSwitch(uint32_t id, uint32_t nq1, double r1,
                     uint32_t nq2, double r2, uint32_t type)
  : Switch(id, AGG_SWITCH) {

  for (uint32_t i = 0; i < nq1; i++) {
    queues.push_back(Factory::get_queue(i, r1, params.queue_size, type, 0, 3));
  }
  for (uint32_t i = 0; i < nq2; i++) {
    queues.push_back(Factory::get_queue(i, r2, params.queue_size, type, 0, 1));
  }
}
