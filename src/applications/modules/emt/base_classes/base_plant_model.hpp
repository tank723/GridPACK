/*
 *     Copyright (c) 2013 Battelle Memorial Institute
 *     Licensed under modified BSD License. A copy of this license can be found
 *     in the LICENSE file in the top level directory of this distribution.
 */
// -------------------------------------------------------------
/**
 * @file   base_plant_model.hpp
 * 
 * @brief  Base model for plant controller
 * 
 * 
 */

#ifndef _base_emt_plant_model_h_
#define _base_emt_plant_model_h_

#include "boost/smart_ptr/shared_ptr.hpp"
#include "gridpack/component/base_component.hpp"
#include <gridpack/applications/modules/emt/constants.hpp>
#include <gridpack/applications/modules/emt/emtutilfunctions.hpp>
#include <gridpack/math/matrix.hpp>
#include <gridpack/applications/modules/emt/base_classes/base_gen_model.hpp>
#include <gridpack/math/dae_solver.hpp>

class BaseEMTGenModel; // Forward declaration for BaseGenModel
class BaseEMTExcModel; // Forward declaration for BaseExcModel

class BaseEMTPlantControllerModel : public gridpack::component::BaseComponent
{
  /* Inheriting the BaseComponent class allows use of functions
     for loading data and accessing/setting values in the vector/matrix
  */
public:
  /**
   * Basic constructor
   */
  BaseEMTPlantControllerModel();
  
  /**
   * Basic destructor
   */
  virtual ~BaseEMTPlantControllerModel();

  /**
     Prestep function
  */
  virtual void preStep(double time ,double timestep) { }
  
  /**
     Poststep function
  */
  virtual void postStep(double time) { }

  /**
    Number of variables
  */ 
  virtual void getnvar(int *nvar) { *nvar = nxplant; }

  /**
     Get the plant controller output
  */
  virtual void getPrefQext(double *Pref, double *Qext) { }

  /**
     Note: This is a custom version of the load method from the BaseComponent Class. It takes in an extra argument idx to specify which component is being read. Ideally, this method should be moved to the MatVecInterface

   * Load data from DataCollection object into corresponding
   * component. This needs to be implemented by every component
   * @param data data collection associated with component
   */
  virtual void load(const boost::shared_ptr<gridpack::component::DataCollection> data, int idx);

  /**
   * Initialize exciter model before calculation
   * @param [output] values - array where initialized exciter variables should be set
   */
  virtual void init(gridpack::RealType *values);
  
  /**
   * Write output from exciters to a string.
   * @param string (output) string with information to be printed out
   * @param bufsize size of string buffer in bytes
   * @param signal an optional character string to signal to this
   * routine what about kind of information to write
   * @return true if bus is contributing string to output, false otherwise
   */
  virtual bool serialWrite(char *string, const int bufsize,
			   const char *signal);
  
  /**
   * Write out exciter state
   * @param signal character string used to determine behavior
   * @param string buffer that contains output
   */
  virtual void write(const char* signal, char* string);

  /**
     set exciter status
  **/
  void setStatus(int estatus) {status = estatus;}
  
  /**
   * return the bolean indicating whether the exciter is ON or OFF
   */
  bool getStatus() {return status;}
  
  /**
   * set current time
   */
  void setTime(double time) {p_time = time; }

  /**
   * Set TSshift: This parameter is passed by PETSc and is to be used in the Jacobian calculation only.
   */
  void setTSshift(double inshift) {shift = inshift;}

  /**
   * Set bus voltage
   */
  void setVoltage(double busVD, double busVQ) {VD = busVD; VQ = busVQ; }

  /**
   * Copy over initial bus voltage from the bus (power flow solution)
   */
  void setInitialVoltage(double inVm,double inVa) {p_Vm0 = inVm; p_Va0 = inVa;}

  virtual void setEvent(gridpack::math::RealDAESolver::EventManagerPtr);

  /**
   * Set the offset for first variable for the exciter in the array for all bus variables 
   * @param offset offset
   */
  void setBusOffset(int offset) {offsetb = offset;}


  /**
   * Get number of matrix values contributed by generator
   * @return number of matrix values
   */
  virtual int matrixNumValues();

  /**
   * Return values from Jacobian matrix
   * @param nvals: number of values to be inserted
   * @param values: pointer to matrix block values
   * @param rows: pointer to matrix block rows
   * @param cols: pointer to matrix block cols
   */
  virtual void matrixGetValues(int *nvals, gridpack::RealType *values, int *rows, int *cols);


  /**
   * Return vector values from the generator model 
   * @param values - array of returned values
   *
   * Note: This function is used to return the entries in vector,
   * for e.g., the entries in the residual vector from the generator
   * object
   */
  virtual void vectorGetValues(gridpack::RealType *values);

  /**
   * Pass solution vector values to the generator object
   * @param values - array of returned values
   *
   * Note: This function is used to pass the entries in vector
   * to the generator object,
   * for e.g., the state vector values for this generator
   */
  virtual void setValues(gridpack::RealType *values);
  
  /**
   * Set the generator associated with this exciter
   **/
  void setGenerator(BaseEMTGenModel* generator) {p_gen = generator; }

  /**
   * Set the electrical controller associated with this plant controller
   */
  void setElectricalController(BaseEMTExcModel* econtroller) {p_econ = econtroller; }

  /**
   * Get the generator associated with this plant controller
   */
  BaseEMTGenModel* getGenerator() { return p_gen; }

  /**
   * Get the electrical controller associated with this plant controller
   */
  BaseEMTExcModel* getElectricalController() { return p_econ; }


  /**
   * Set an internal variable that can be used to control the behavior of the
   * component. This function doesn't need to be implemented, but if needed,
   * it can be used to change the behavior of the component in different phases
   * of the calculation. For example, if a different matrix needs to be
   * generated at different times, the mode of the calculation can changed to
   * get different values from the MatVecInterface functions
   * @param mode integer indicating which mode should be used
   */
  void setMode(int mode) { p_mode = mode;}

  void setBusLocalOffset(int offset) {p_busoffset = offset;}

  /*
    set the location for the first variable in the solution vector
  */
  void setGlobalLocation(int gloc) {p_gloc = gloc;}

  /*
    set the global location for first voltage variable for this bus
  */
  void setVoltageGlobalLocation(int glocvoltage) {p_glocvoltage = glocvoltage;}


  /**
   * return offset in the local vector 
   */
  int getLocalOffset()
  {
    return p_busoffset + offsetb;
  }

  virtual void resetEventFlags(void) {}

  /**
   * Copy over voltage from the bus
   */
  void setVoltage(double inva, double invb,double invc) {p_va = inva; p_vb = invb; p_vc = invc;}

  /**
   * Set type of integration algorithm
   */
  void setIntegrationType(EMTMachineIntegrationType type) {integrationtype = type; }

protected:
  double        VD, VQ;
  int           status; /**< Plant status */
  double        mbase,sbase; /** Machine base and system MVA base */
  int           busnum; /** Bus number */
  double        p_time = 0.0;   /** Current time */
  double        p_Vm0, p_Va0; /** Initial voltage magnitude and angle **/
  double        p_va,p_vb,p_vc; /** Bus voltage **/
  double        shift; // shift (multiplier) used in the Jacobian calculation
  
  EMTMachineIntegrationType integrationtype; // Integration type 

  BaseEMTGenModel* p_gen; // Generator model
  BaseEMTExcModel* p_econ; // Electrical Controller model
  
  int           offsetb; /**< offset for the first variable for the generator in the array for all bus variables */
  int           p_gloc; // Global location of the first variable for the generator
  int           p_glocvoltage; // Global location for the first voltage variable for the bus

  int           nxplant;    /** Number of variables for the model */
  int           p_busoffset; /** Starting location for bus variables in the local state vector */
  int           p_nrows;  // number of rows (equations) contributed by this excitor
  int           p_ncols;  // number of columns (variables) contributed by this excitor
  std::vector<int>   p_rowidx; // global index for rows
  std::vector<int>   p_colidx; // global index for columns

};

#endif
