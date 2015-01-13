#include <iostream>
#include <algorithm>
#include <fstream>
#include <stdlib.h>
#include <deque>
#include <stdint.h>
#include <time.h>

#include "flow.h"
#include "turboflow.h"


#include "packet.h"
#include "node.h"
#include "event.h"
#include "topology.h"
#include "simpletopology.h"
#include "params.h"
#include "assert.h"
#include "queue.h"

#include "factory.h"
#include "random_variable.h"

using namespace std;


Topology *topology;
double current_time = 0;
std::priority_queue<Event *, std::vector<Event *>, EventComparator> event_queue;
std::deque<Flow *> flows_to_schedule;
std::deque<Event *> flow_arrivals;

uint32_t num_outstanding_packets;
uint32_t max_outstanding_packets;

extern DCExpParams params;
extern double get_current_time(); // TODO
extern void add_to_event_queue(Event *);
extern int get_event_queue_size();
uint32_t duplicated_packets_received = 0;
double start_time = -1;

/* Read parameters from a config file */
void read_experiment_parameters(std::string conf_filename, uint32_t exp_type) {
  std::ifstream input(conf_filename);

  std::string temp;
  input >> temp; input >> params.initial_cwnd;
  input >> temp; input >> params.max_cwnd;
  input >> temp; input >> params.retx_timeout_value;
  input >> temp; input >> params.queue_size;
  input >> temp; input >> params.propagation_delay;
  input >> temp; input >> params.bandwidth;
  input >> temp; input >> params.queue_type;
  input >> temp; input >> params.flow_type;

  input >> temp;

  if (exp_type == DC_EXP_CONTINUOUS_FLOWMODEL) {
    input >> params.end_time;
  }
  else {
    input >> params.num_flows_to_run;
  }
  input >> temp; input >> params.cdf_or_flow_trace;
  input >> temp; input >> params.cut_through;
  input >> temp; input >> params.mean_flow_size;
  input >> temp; input >> params.load_balancing;
  input >> temp; input >> params.preemptive_queue;



  params.hdr_size = 40;
  params.mss = 1460;
}


/* Runs a initialized scenario */
void run_scenario() {
  // Flow Arrivals create new flow arrivals
  // Add the first flow arrival
  if (flow_arrivals.size() > 0) {
    add_to_event_queue(flow_arrivals.front());
    flow_arrivals.pop_front();
  }
  while (event_queue.size() > 0) {
    Event *ev = event_queue.top();
    event_queue.pop();
    current_time = ev->time;
    if (start_time < 0) {
      start_time = current_time;
    }
    //event_queue.pop();
    //std::cout << "main.cpp::run_scenario():" << get_current_time() << " Processing " << ev->type << " " << event_queue.size() << std::endl;
    if (ev->cancelled) {
      delete ev; //TODO: Smarter
      continue;
    }
    ev->process_event();
    delete ev;
  }
}


extern void run_pFabric_experiment(int argc, char **argv, uint32_t exp_type);
extern void run_continuous_experiment(int argc, char **argv);
extern void run_fixedDistribution_experiment(int argc, char **argv, uint32_t exp_type);
extern void run_fixedDistribution_experiment_shuffle_traffic(int argc, char **argv, uint32_t exp_type);
extern void run_single_link_experiment(int argc, char ** argv);
extern void run_single_sender_receiver_exp(int argc, char ** argv);
extern void run_nto1_experiment(int argc, char ** argv);

int main (int argc, char ** argv) {
  time_t start_time;
  time(&start_time);

  //srand(time(NULL));
  srand(0);
  std::cout.precision(15);

  uint32_t exp_type = atoi(argv[1]);
  switch (exp_type) {
    case DC_EXP_WITH_TRACE:
    case DC_EXP_WITHOUT_TRACE:
      run_pFabric_experiment(argc, argv, exp_type);
      break;
    case DC_EXP_CONTINUOUS_FLOWMODEL:
      run_continuous_experiment(argc, argv);
      break;
    case DC_EXP_UNIFORM_FLOWS:
      run_fixedDistribution_experiment(argc, argv, exp_type);
      break;
    case DC_EXP_UNIFORM_FLOWS_SHUFFLE_TRAFFIC:
      run_fixedDistribution_experiment_shuffle_traffic(argc, argv, exp_type);
      break;
    case SINGLE_LINK_EXP_IONSTYLE:
      run_single_link_experiment(argc, argv);
      break;
    case SINGLE_LINK_EXP_SYLVIASTYLE:
      run_single_sender_receiver_exp(argc, argv);
      break;
    case N_TO_1_EXP:
      run_nto1_experiment(argc, argv);
      break;
  }

  time_t end_time;
  time(&end_time);
  double duration = difftime(end_time, start_time);
  cout << "Simulator ended. Execution time: " << duration << " seconds\n";
}