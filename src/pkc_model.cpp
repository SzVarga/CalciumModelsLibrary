#include "CaModLib_global_vars.hpp"
#include <Rcpp.h>
using namespace Rcpp;


/* Global variables */
static int nspecies = 11;
static int nreactions = 20;
static double k1 = 1;
static double k2 = 50;
static double k3 = 1.2e-7;
static double k4 = 0.1;
static double k5 = 1.2705;
static double k6 = 3.5026;
static double k7 = 1.2e-7;
static double k8 = 0.1;
static double k9 = 1;
static double k10 = 0.1;
static double k11 = 2;
static double k12 = 0.2;
static double k13 = 0.0006;
static double k14 = 0.5;
static double k15 = 7.998e-6;
static double k16 = 8.6348;
static double k17 = 6e-7;
static double k18 = 0.1;
static double k19 = 1.8e-5;
static double k20 = 2;
static double AA = 11000;  // given as conc. remains fixed throughout the simulation
static double DAG = 5000;  // given as conc. remains fixed throughout the simulation

// Define (initialize) variables declared in header
// Pointers *amu and *x are initialized with the adress of filler variables
// (These values are overwritten by the simulation)
static NumericVector calcium(1);
static unsigned int ntimepoint = 0;
static double amu_init_value = 0.1;
static double *amu = &amu_init_value;
static unsigned long long int x_init_value = 1000000;
static unsigned long long int *x = &x_init_value; 


//' Propensity Calculation
//'
//' Calculates the propensities of all PKC model reactions and stores them in the vector amu.
//'
//' @param None
//' @return None
//' @examples
//' calculate_amu() 
//' @export
// [[Rcpp::export]]
void pkc_calculate_amu() {

  amu[0] = k1 * x[0];
  amu[1] = amu[0] + k2 * x[5];
  amu[2] = amu[1] + k3 * AA * (double)x[0]; /* AA given as conc., hence, no scaling */
  amu[3] = amu[2] + k4 * x[6];
  amu[4] = amu[3] + k5 * x[1];
  amu[5] = amu[4] + k6 * x[7];
  amu[6] = amu[5] + k7 * AA * (double)x[1];  /* AA given as conc., hence, no scaling */
  amu[7] = amu[6] + k8 * x[8];
  amu[8] = amu[7] + k9 * x[2];
  amu[9] = amu[8] + k10 * x[9];
  amu[10] = amu[9] + k11 * x[3];
  amu[11] = amu[10] + k12 * x[4];
  amu[12] = amu[11] + calcium[ntimepoint] * k13 * (double)x[0]; /* Ca given as conc., hence, no scaling */
  amu[13] = amu[12] + k14 * x[1];
  amu[14] = amu[13] + k15 * DAG * (double)x[1]; /* DAG given as conc., hence, no scaling */
  amu[15] = amu[14] + k16 * x[2];
  amu[16] = amu[15] + k17 * DAG * (double)x[0]; /* DAG given as conc., hence, no scaling */
  amu[17] = amu[16] + k18 * x[10];
  amu[18] = amu[17] + k19 * AA * (double)x[10];  /* AA given as conc., hence, no scaling */
  amu[19] = amu[18] + k20 * x[3];
}

//' System Update
//'
//' Changes the system state (updates the particle numbers) by instantiating a chosen reaction.
//'
//' @param rIndex An unsigned integer: the id of the chosen reaction.
//' @return void
//' @examples
//' update_system() 
//' @export
// [[Rcpp::export]]
void pkc_update_system(unsigned int rIndex) {
  
  switch (rIndex) {
  case 0:   /* R1 */
    x[0]--;
    x[5]++;
    break;
  case 1:
    x[5]--;
    x[0]++;
    break;
  case 2:   /* R2 */
    x[0]--;
    x[6]++;
    break;
  case 3:
    x[6]--;
    x[0]++;
    break;
  case 4:  /* R3 */
    x[1]--;
    x[7]++;
    break;
  case 5:
    x[7]--;
    x[1]++;
    break;
  case 6:  /* R4 */
    x[1]--;
    x[8]++;
    break;
  case 7:
    x[8]--;
    x[1]++;
    break;
  case 8: /* R5 */
    x[2]--;
    x[9]++;
    break;
  case 9:
    x[9]--;
    x[2]++;
    break;
  case 10:/* R6 */
    x[3]--;
    x[4]++;
    break;
  case 11:
    x[4]--;
    x[3]++;
    break;
  case 12:/* R7 */
    x[0]--;
    x[1]++;
    break;
  case 13:
    x[1]--;
    x[0]++;
    break;
  case 14:/* R8 */
    x[1]--;
    x[2]++;
    break;
  case 15:
    x[2]--;
    x[1]++;
    break;
  case 16:/* R9 */
    x[0]--;
    x[10]++;
    break;
  case 17:
    x[10]--;
    x[0]++;
    break;
  case 18:/* R10 */
    x[10]--;
    x[3]++;
    break;
  case 19:
    x[3]--;
    x[10]++;
    break;
    printf("\nError in updateSystem(): rIndex (%u) out of range!\n", rIndex);
    exit(-1);
  }
}