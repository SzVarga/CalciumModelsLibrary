#include <string>
#include <Rcpp.h>
using namespace Rcpp;


//********************************/* R EXPORT OPTIONS */********************************

// 1. USER INPUT for new models: Change value of the macro variable MODEL_NAME to the name of the new model.
#define MODEL_NAME calmodulin
// include the simulation function with macros (#define statements) that make it model specific (based on MODEL_NAME)
#include "simulator.cpp"
// Global variables
static std::map <std::string, double> prop_params_map;
// 2. USER INPUT for new models: Change the name of the wrapper function to sim_<MODEL_NAME> and the names of the internally called functions to init_<MODEL_NAME> and simulator_<MODEL_NAME>.
//' Calmodulin Model R Wrapper Function (exported to R)
//'
//' This function calls the internal C++ simulator function to simulate the Calmodulin model. 
//' @param
//' @return
//' @examples
//' sim_calmodulin()
//' @export
// [[Rcpp::export]]
NumericMatrix sim_calmodulin(DataFrame user_input_df,
                   NumericVector user_sim_params,
                   List user_model_params) {

  // Extract vectors from user supplied list
  NumericVector user_vols = user_model_params["vols"];
  NumericVector user_init_conc = user_model_params["init_conc"];
  NumericVector user_params = user_model_params["params"];
  // Provide default model parameters list (Rcpp type)
  List default_model_params = init_calmodulin();
  // Extract default vectors
  NumericVector default_vols = default_model_params["vols"];
  NumericVector default_init_conc = default_model_params["init_conc"];
  NumericVector default_params = default_model_params["params"];
  // Model Parameter Check: replace entries in default_model_params with user-supplied values if necessary
  // Volumes check:
  CharacterVector user_vols_names = user_vols.names();
  for (int i = 0; i < user_vols_names.length(); i++) {
    std::string current_vol_name = as<std::string>(user_vols_names[i]);
    if (user_vols.containsElementNamed((current_vol_name).c_str())) {
      default_vols[current_vol_name] = user_vols[current_vol_name];    
    }
  }
  // Initial conditions check:
  CharacterVector user_init_conc_names = user_init_conc.names();
  for (int i = 0; i < user_init_conc_names.length(); i++) {
    std::string current_init_conc_name = as<std::string>(user_init_conc_names[i]);
    if (user_init_conc.containsElementNamed((current_init_conc_name).c_str())) {
      default_init_conc[current_init_conc_name] = user_init_conc[current_init_conc_name];    
    }
  }  
  // Propensity equation parameters check:
  CharacterVector user_params_names = user_params.names();
    for (int i = 0; i < user_params_names.length(); i++) {
    std::string current_param_name = as<std::string>(user_params_names[i]);
    if (user_params.containsElementNamed((current_param_name).c_str())) {
      default_params[current_param_name] = user_params[current_param_name];    
    }
  }
  // Put propensity reaction parameters in a map (for function calculate_amu)
  // (take params from vector default_params which contains the updated values after check)
  CharacterVector default_params_names = default_params.names();
  for (int n = 0; n < default_params.length(); n++) {
    std::string current_param_name = as<std::string>(default_params_names[n]);
    prop_params_map[current_param_name] = default_params[current_param_name];  
  }
  
  // Return result of the simulation
  return simulator_calmodulin(user_input_df,
                   user_sim_params,
                   default_vols,
                   default_init_conc);
   
}



//********************************/* MODEL DEFINITION */********************************
// 3. USER INPUT for new models: define model parameters, number of species, 
// number of reactions, propensity equations and update_system function 

// Default model parameters
List init() {
  // Model dimensions
  nspecies = 2;
  nreactions = 2;
    
  // Default volume(s)
  NumericVector vols = NumericVector::create(
    _["vol"] = 5e-14
  );
  // Default initial conditions
  NumericVector init_conc = NumericVector::create(
    _["Prot_inact"] = 5.0,
    _["Prot_act"] = 0
  );
  // Default propensity equation parameters
  NumericVector params = NumericVector::create(
    _["k_on"] = 0.025,
    _["k_off"] = 0.005,
    _["Km"] = 1.0,
    _["h"] = 4.0
  );
    
  // Combine and return all vectors in a default_params list
  return List::create(
    _["vols"] = vols,
    _["init_conc"] = init_conc,
    _["params"] = params
  );
}

// Propensity calculation:
// Calculates the propensities of all Calmodulin model reactions and stores them in the vector amu.
void calculate_amu() {
  
  // Look up model parameters in array 'prop_params_map' initially
  // (contains updated default parameters from vector default_params)
  double k_on = prop_params_map["k_on"];
  double k_off = prop_params_map["k_off"];
  double Km = prop_params_map["Km"];
  double h = prop_params_map["h"];
  
  amu[0] = ((k_on * pow((double)calcium[ntimepoint],(double)h)) / (pow((double)Km,(double)h) + pow((double)calcium[ntimepoint],(double)h))) * x[0];
  amu[1] = amu[0] + k_off * x[1];
    
}

// System update:
// Changes the system state (updates the particle numbers) by instantiating a chosen reaction.
void update_system(unsigned int rIndex) {
  switch (rIndex) {
  case 0:   // Activation
    x[0]--;
    x[1]++;
    break;
  case 1:   // Deactivation
    x[0]++;
    x[1]--;
    break;
    printf("\nError in updateSystem(): rIndex (%u) out of range!\n", rIndex);
    exit(-1);
  }
}