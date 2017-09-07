#include "extern_simulator_func_prototype.hpp"
#include <Rcpp.h>
using namespace Rcpp;


// The general simulator file is included in every C++ model file -> if they provide a MODEL_NAME, all functions will be renamed in a model specific fashion
#ifdef MODEL_NAME
  #define Map_helper(x,y) x##y
  #define Map(x,y) Map_helper(x,y)
  #define simulator Map(simulator_, MODEL_NAME)
  #define init Map(init_, MODEL_NAME)
  #define calculate_amu Map(calculate_amu_, MODEL_NAME)
  #define update_system Map(update_system_, MODEL_NAME)

  // Placeholder init function since the R Wrapper Function tries to call it before its 'real' definition in the C++ model file
  std::map <std::string, double> init();
#endif


// Global shared variables
extern NumericVector timevector;
extern double timestep;
extern double vol;
extern NumericVector calcium;
extern unsigned int ntimepoint;
extern double *amu;
extern unsigned long long int *x;
extern int nspecies;
extern int nreactions;
// Global shared functions
extern void calculate_amu();
extern void update_system(unsigned int rIndex);


//' Stochastic Simulator (Gillespie's Direct Method).
//'
//' Simulate a calcium dependent protein coupled to an input calcium time series using an implementation of Gillespie's Direct Method SSA.
//'
//' @param user_input_df A data frame: contains the times of the observations (column "time") and the cytosolic calcium concentration [nmol/l] (column "Ca").
//' @param user_sim_params A numeric vector: contains all simulation parameters ("timestep": the time interval between two output samples, "endTime": the time at which to end the simulation).
//' @param user_model_params A list: contains all model parameters ("vol": the volume of the system [l], "init_conc": the initial concentrations of model species [nmol/l] and possibly other specific parameters that have been compared to the default parameter values in the C++ model file).
//' @return A dataframe with time and the active protein time series as columns.
//' @examples
//' simulator()
NumericMatrix simulator(DataFrame user_input_df,
                   NumericVector user_sim_params,
                   List user_model_params,
                   std::map <std::string, double> model_params) {

  // get R random generator state
  GetRNGstate();
  
  /* VARIABLES */
  // Get parameter values from arguments
  timevector = user_input_df["time"];
  calcium = user_input_df["Ca"];
  timestep = user_sim_params["timestep"];
  vol = as<double>(user_model_params["vol"]);
  // identify initial conditions in model_params map:
  // they are always at the end of the map
  // and there are as many initial conditions as there are species
  // => the initial conditions start at map.size()-nspecies
  size_t map_pos = 0;
  int ic_index = 0;
  NumericVector ic(nspecies);
  for (std::map<std::string, double>::iterator iter = model_params.begin(); 
       iter != model_params.end(); ++iter) {
    
    std::string curr_key = iter->first;
    Rcout << "Current Map Key: " << curr_key << std::endl;
    Rcout << "Current Map Element: " << model_params[curr_key] << std::endl;
    
    if (map_pos >= model_params.size()-nspecies && map_pos < model_params.size()) {
      std::string key = iter->first;
      ic[ic_index] = model_params[key];
      ic_index++;
    }
    map_pos++;
  }
  
  
  Rcout << "MAP Prot_inact: " << model_params["Prot_inact"] << std::endl;
  Rcout << "MAP Prot_act: " << model_params["Prot_act"] << std::endl;

  Rcout << "IC Prot_inact: " << ic[0] << std::endl;
  Rcout << "IC Prot_act: " << ic[1] << std::endl;  
  
  // Particle number <-> concentration (nmol/l) factor (n/f = c <=> c*f = n)
  double f;
  f = 6.0221415e14*vol;
  // Control variables
  int noutput;
  ntimepoint = 0;
  noutput = 0;
  int xID;
  // Variables for random steps
  double tau;
  double r2;
  unsigned int rIndex;
  // Time variables
  double startTime;
  double endTime;
  double currentTime;
  double outputTime;
  startTime = timevector[0];
  endTime = user_sim_params["endTime"];
  currentTime = startTime;
  outputTime = currentTime;
  // Return value
  int nintervals = (int)floor((endTime-startTime)/timestep+0.5)+1;
  NumericMatrix retval(nintervals, nspecies+2); // nspecies+2 because time and calcium
  // Memory allocation for propensity and particle number pointers
  amu = (double *)Calloc(nreactions, double);
  x = (unsigned long long int *)Calloc(nspecies, unsigned long long int);
  // Initial particle numbers
  int i;
  for (i=0; i < ic.length(); i++) {
    x[i] = (unsigned long long int)floor(ic[i]*f);  
  }
  
  Rcout << "X Prot_inact: " << x[0] << std::endl;
  Rcout << "X Prot_act: " << x[1] << std::endl;
  
  /* SIMULATION LOOP */
  while (currentTime < endTime) {
    R_CheckUserInterrupt();
    // Calculate propensity amu for every reaction
    calculate_amu();
    // Calculate time step tau
    tau = - log(runif(1)[0])/amu[nreactions-1];
    // Check if reaction time exceeds time until the next observation 
    if ((currentTime+tau)>=timevector[ntimepoint+1]) {
      // Set current simulation time to next timepoint in input calcium time series
      currentTime = timevector[ntimepoint+1];
      // Update output
      while ((currentTime > outputTime)&&(outputTime < endTime)) {
        retval(noutput, 0) = outputTime;
        retval(noutput, 1) = calcium[ntimepoint];
        for (xID=2; xID < 2+nspecies; xID++) {
          retval(noutput, xID) = x[xID-2]/f;
        }
        noutput++;
        outputTime += timestep;
      }
      ntimepoint++;
    } else {
      // Select reaction to fire
      r2 = amu[nreactions-1] * runif(1)[0];
      rIndex = 0;
      for (rIndex=0; amu[rIndex] < r2; rIndex++);
      // Propagate time
      currentTime += tau;
     // Update output
      while ((currentTime > outputTime)&&(outputTime < endTime)) {
        retval(noutput, 0) = outputTime;
        retval(noutput, 1) = calcium[ntimepoint];
        for (xID=2; xID < 2+nspecies; xID++) {
          retval(noutput, xID) = x[xID-2]/f;
        }
        noutput++;
        outputTime += timestep;
      }
      // Update system state
      update_system(rIndex);
    }
  }
  // Update output
  while (floor(outputTime*10000) <= floor(endTime*10000)) {
    retval(noutput, 0) = outputTime;
    retval(noutput, 1) = calcium[ntimepoint];
    for (xID=2; xID < 2+nspecies; xID++) {
      retval(noutput, xID) = x[xID-2]/f;
    }
    noutput++;
    outputTime += timestep;
  }
     
  // Free dyn. allocated pointers
  Free(amu);
  Free(x);
  
  // Send random generator state back to R
  PutRNGstate();
  
  return retval;
}
