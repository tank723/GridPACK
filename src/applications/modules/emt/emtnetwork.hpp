/*
 *     Copyright (c) 2013 Battelle Memorial Institute
 *     Licensed under modified BSD License. A copy of this license can be found
 *     in the LICENSE file in the top level directory of this distribution.
 */
// -------------------------------------------------------------
/**
 * @file   emtnetwork.hpp
 *
 * @brief  
 * Network component definitions
 * 
 */
// -------------------------------------------------------------

#ifndef _emtnetwork_h_
#define _emtnetwork_h_

#include <boost/smart_ptr/shared_ptr.hpp>
#include <gridpack/include/gridpack.hpp>
#include <gridpack/utilities/complex.hpp>
#include <constants.hpp>
#include <base_classes/base_gen_model.hpp>
#include <base_classes/base_exc_model.hpp>
#include <base_classes/base_gov_model.hpp>
#include <base_classes/base_load_model.hpp>
#include <gridpack/math/dae_solver.hpp>
#include <emtutilfunctions.hpp>

class EmtBus: public gridpack::component::BaseBusComponent 
{
public:
  /**
   *  Simple constructor
   */
  EmtBus(void);
  
  /**
   *  Simple destructor
   */
  ~EmtBus(void);
  
  /**
   *  Check if the bus is isolated. Returns true if the bus is isolated
   
   */
  bool isIsolated(void) const;
    
  /**
   * Load values stored in DataCollection object into DSBus object. The
   * DataCollection object will have been filled when the network was created
   * from an external configuration file
   * @param data: DataCollection object contain parameters relevant to this
   *       bus that were read in when network was initialized
   */
  void load(const boost::shared_ptr<gridpack::component::DataCollection> &data);
  
  /**
   * Set the model to control what matrices and vectors and built when using the
   * mapper
   * @param mode: enumerated constant for different modes
   */
  void setMode(int mode);
  
  /**
   * Return number of rows (equations) that bus contributes
   * to matrix
   * @return number of dependent variables (equations)
   */
  int matrixNumRows();

  /**
   * Return number of columns (variables) that bus contributes
   * to matrix
   * @return number of independent variables
   */
  int matrixNumCols();
  
  /** 
   * Set global row index
   * @param irow local row index
   * @param global row index
   */
  void matrixSetRowIndex(int irow, int idx);

  /** 
   * Set global column index
   * @param icol local column index
   * @param global column index
   */
  void matrixSetColIndex(int icol, int idx);

  /**
   * Return global row index given local row index
   * @param irow local row index
   * @return global row index
   */
  int matrixGetRowIndex(int irow);
  
  /**
   * Return global column index given local column index
   * @param icol local column index
   * @return global column index
   */
  int matrixGetColIndex(int icol);

  /**
   * Total number of matrix elements returned by bus
   * @return number of matrix elements
   */
  int matrixNumValues();

  /**
   * Return list of matrix values and their locations generated by the bus
   * @params nvals number of values inserted
   * @param values list of matrix values
   * @param rows list of row indices
   * @param cols list of column indices
   */
  void matrixGetValues(int *nvals,gridpack::ComplexType *values,
      int *rows, int *cols);

  /**
   */
  void matrixGetValues(gridpack::math::Matrix &matrix);
  
  /**
   * Set value of global index for corresponding local index
   * @param ielem local index for element
   * @param idx global index of element
   */
  void vectorSetElementIndex(int ielem, int idx);

  /**
   * Return a set of element indices that map the local indices to
   * global indices
   * @param idx array of global indices
   */
  void vectorGetElementIndices(int *idx);

  /**
   * Return number elements contributed by this bus
   * @return number of elements
   */
  int vectorNumElements() const;

  /**
   * Return the elements and their global indices in the vector
   * @param values array of element values
   * @param idx array of element indices
   */
  void vectorGetElementValues(gridpack::ComplexType *values, int *idx);

  /**
   * Set network elements based on values in vector
   * @param array containing vector values
   */
  void vectorSetElementValues(gridpack::ComplexType *values);
  
  /**
   * Write output from buses to standard out
   * @param string (output) string with information to be printed out
   * @param bufsize size of string buffer in bytes
   * @param signal an optional character string to signal to this
   * routine what about kind of information to write
   * @return true if bus is contributing string to output, false otherwise
   */
  bool serialWrite(char *string, const int bufsize, const char *signal = NULL);
  
  /**
   * Get the phase voltages from the exchange buffer
   * @param double va - phase a voltage
   * @param double vb - phase b voltage
   * @param double vc - phase c voltage
   */
  void getVoltages(double*,double*,double*) const;

  /**
   *  getInitialVoltage - Get the initial voltage
   *  @param[output] double Vm - voltage magnitude
   *  @param[output] double Va - voltage angle
   *
   * Returns the initial (t=0) voltage magnitude and angle for the bus
   */
  void getInitialVoltage(double *Vm, double *Va) { *Vm = p_Vm0; *Va = p_Va0;}
  
  /**
     Set the shift value provided by TS
  */
  void setTSshift(double);

  /**
     Set the current time provided by TS
  */
  void setTime(double);

  /**
     Set buffer size for exchange
  */
  int getXCBufSize(void);
  
  void setXCBuf(void*);

  /*
    setup - set up routine for the bus component
  */
  void setup(void);
  
  /* Set whether the element is local or ghosted */
  void setGhostStatus(bool isghost) { p_isghost = isghost; }
  
  bool isGhost(void) { return p_isghost; }
  
  void setRank(int rank) { p_rank = rank; }

  void setEvent(gridpack::math::DAESolver::EventManagerPtr);

  void setLocalOffset(int offset);

  void resetEventFlags(void);

  /**
    AddLumpedLineCshunt - Add capacitive shunt of lumped line model
                         at this bus
    @param [input] Cshunt - the capacitive shunt matrix 3 X 3
    @param [input] frac   - the fraction of capacitive shunt that should
                            be added
    Note: The capacitive shunt added to the bus equals frac*Cshunt
  **/
     
  void addLumpedLineCshunt(double Cshunt[3][3], double frac)
  {
    int i,j;
    for(i=0; i < 3; i++) {
      for(j=0; j < 3; j++) {
	p_Cshunt[i][j] += frac*Cshunt[i][j];
      }
    }
  }
  
private:
  // Anything declared here should be set in the Archive class in exactly the same order!!
  // Data needed for calculations
  double p_sbase;   // System MVA base
  int    p_bustype; // Bus type
  double p_gl,p_bl; // Shunt conductance and susceptance (p.u.)
  double p_pl,p_ql; // Active and reactive load (p.u)
  double p_Vm0,p_Va0;     // Voltage magnitude and angle at t=0 used for building constant impedance load
  int    p_ngen;    // Number of generators incident on this bus
  int    p_nload;   // Number of loads incident on this bus
  int    p_nactivegen; // Number of active generators (status=1) on this bus
  bool   p_isolated;   // flag for isolated bus
  EMTMode p_mode; // factory mode
  double p_time;     // current time
  double p_TSshift;  // shift value provided by TSIJacobian. 
  int    p_nvar;      // Total number of variables for this bus (includes gen, exc, etc.)
  int    p_nvarbus;   // Only the variables for the bus (no component variables included)
  int    p_offset; // Offset for the starting location for this bus's variables in the state vector (array)

  // EMT data
  double p_nphases;
  double p_Cshunt[3][3]; // Shunt capacitance 3X3 block
  double p_Cshuntinv[3][3]; // Inverse of the shunt capacitance
  // The shunt capacitance is a combination of the bus shunt capacitance
  // and the lumped parameter line capacitance p_C = p_Cshunt + \sum_i^lines_connected{p_Cline_i/2}
  double p_Gshunt[3][3]; // Shunt resistance 3X3 block
  double p_Lshunt[3][3]; // Shunt inductance 3X3 block
  bool   p_hasCapacitiveShunt; // Does the bus have capacitive shunt
  bool   p_hasInductiveShunt; // Does the bus have inductive shunt
  bool   p_hasResistiveShunt; // Does the bus have resistive shunt

  // Generalized mapper interface
  std::vector<int>    p_rowidx;   // array holding row indices
  std::vector<int>    p_colidx;   // array holding column indices
  int                *p_vecidx;   // array holding vector indices
  int                 p_num_vals; // total number of matrix elements returned by bus
  
  // EMT Variables
  double *p_vptr; // Pointer used for exchanging values with ghost buses. Note that this pointer is pointed to the buffer used for exchanging values with ghost buses. Its contents should be updated whenever there is a change in v, e.g., when the values from vector X are being mapped to the buses

  double p_dvdt[3]; // Voltage derivatives returned by DAE solver
  bool   p_isghost; // Is it a local or ghosted element
  
  int    p_rank;
  
  BaseEMTGenModel **p_gen;    // Generator model
  BaseEMTLoadModel **p_load;  // Load model
  int           *p_neqsload; // Number of equations for each load
  int           *p_neqsgen; // Number of equations for each generator
  int           *p_neqsexc; // Number of equations for each exciter 
  int           *p_neqsgov; // Number of equations for each governor 
  
  friend class boost::serialization::access;
  
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & boost::serialization::base_object<gridpack::component::BaseBusComponent>(*this)
      & p_sbase
      & p_bustype
      & p_gl & p_bl
      & p_pl & p_ql
      & p_Vm0 & p_Va0
      & p_ngen
      & p_nload
      & p_nactivegen
      & p_isolated
      & p_mode
      & p_time
      & p_TSshift
      & p_nvar
      & p_offset
      & p_isghost
      & p_rank;
  }  
  
}; // End of EmtBus

class EmtBranch
  : public gridpack::component::BaseBranchComponent {
public:
  /**
   *  Simple constructor
   */
  EmtBranch(void);
  
  /**
   *  Simple destructor
   */
  ~EmtBranch(void);
  
  /**
   * Load values stored in DataCollection object into EmtBranch object. The
   * DataCollection object will have been filled when the network was created
   * from an external configuration file
   * @param data: DataCollection object contain parameters relevant to this
   *       branch that were read in when network was initialized
   */
  void load(const boost::shared_ptr<gridpack::component::DataCollection> &data);
  
  /**
   * Set the model to control what matrices and vectors and built when using the
   * mapper
   * @param mode: enumerated constant for different modes
   */
  void setMode(int mode);

  /**
     Set the shift value provided by TS
  */
  void setTSshift(double);

  /**
     Set the current time provided by TS
  */
  void setTime(double);

  /**
   * getCurrent - returns the line current
   *
   * @param[input]  idx - For the nth parallel line number, idx = n. For no parallel lines, idx = 0
   * @param[output] ia - phase a current
   * @param[output] ib - phase b current
   * @param[output] ic - phase c current
   */
  void getCurrent(int idx,double *ia, double *ib, double *ic) {
    double *i = p_ibr + 3*idx;

    *ia = i[0];
    *ib = i[1];
    *ic = i[2];
  }
  
  /**
     Set buffer size for exchange
  */
  int getXCBufSize(void);
  
  void setXCBuf(void*);
  
  /**
   * Return number of rows (dependent variables) that branch contributes
   * to matrix
   * @return number of dependent variables (equations)
   */
  int matrixNumRows();

  /**
   * Return number of columns (independent variables) that branch contributes
   * to matrix
   * @return number of independent variables
   */
  int matrixNumCols();

  /** 
   * Set global row index
   * @param irow local row index
   * @param global row index
   */
  void matrixSetRowIndex(int irow, int idx);

  /** 
   * Set global column index
   * @param icol local column index
   * @param global column index
   */
  void matrixSetColIndex(int icol, int idx);

  /**
   * Return global row index given local row index
   * @param irow local row index
   * @return global row index
   */
  int matrixGetRowIndex(int irow);
  
  /**
   * Return global column index given local column index
   * @param icol local column index
   * @return global column index
   */
  int matrixGetColIndex(int icol);
  
  /**
   * Total number of matrix elements returned by branch
   * @return number of matrix elements
   */
  int matrixNumValues();
  
  /**
   * Return list of matrix values and their locations generated by the branch
   * @params nvals number of values inserted
   * @param values list of matrix values
   * @param rows list of row indices
   * @param cols list of column indices
   */
  void matrixGetValues(int *nvals,gridpack::ComplexType *values,
      int *rows, int *cols);

  void matrixGetValues(gridpack::math::Matrix &matrix);

  /**
   * Set value of global index for corresponding local index
   * @param ielem local index for element
   * @param idx global index of element
   */
  void vectorSetElementIndex(int ielem, int idx);

  /**
   * Return a set of element indices that map the local indices to
   * global indices
   * @param idx array of global indices
   */
  void vectorGetElementIndices(int *idx);

  /**
   * Return number elements contributed by this bus
   * @return number of elements
   */
  int vectorNumElements() const;

  /**
   * Return the elements and their global indices in the vector
   * @param values array of element values
   * @param idx array of element indices
   */
  void vectorGetElementValues(gridpack::ComplexType *values, int *idx);

  /**
   * Set network elements based on values in vector
   * @param array containing vector values
   */
  void vectorSetElementValues(gridpack::ComplexType *values);

  /**
   * Return the number of parallel lines
   */
  int getNumParallelLines(){ return p_nparlines;}
  /**
   * Write output from branches to standard out
   * @param string (output) string with information to be printed out
   * @param bufsize size of string buffer in bytes
   * @param signal an optional character string to signal to this
   * routine what about kind of information to write
   * @return true if branch is contributing string to output, false otherwise
   */
  bool serialWrite(char *string, const int bufsize, const char *signal = NULL);

  /*
    setup - set up routine for the branch component
  */
  void setup(void);


  /*
    setLocalOffset - sets the starting location for the variables for this branch in the solution vector
  */
  void setLocalOffset(int offset) {p_offset = offset;}
  
  /* Set whether the element is local or ghosted */
  void setGhostStatus(bool isghost) { p_isghost = isghost; }
  
  bool isGhost(void) { return p_isghost; }
  
  void setRank(int rank) { p_rank = rank; }

  int  getStatus(int i) {return p_status[i];}
private:
  int p_nparlines; // Number of parallel lines
  std::vector<int> p_status; // Status of the lines
  std::vector<std::string> p_cktid; // circuit id
  std::vector<int> p_localoffset; // local offset used for inserting/retrieving values from vector
  std::vector<double> p_lineR; // Line resistance
  std::vector<double> p_lineX; // Line reactance
  int  p_nvar;      // Number of variables for this branch  
  int p_mode;     // Mode used for vectors and matrices
  double p_time;     // current time
  double p_TSshift;  // shift value provided by TSIJacobian.
  bool p_isghost; // Local or ghosted element
  int  p_rank;   // Process rank
  int p_offset; // Starting location for the variables for this branch in the solution vector

  // Used by generalized mapper interface
  std::vector<int>    p_rowidx;   // array holding row indices
  std::vector<int>    p_colidx;   // array holding column indices
  int                *p_vecidx;   // array holding vector indices
  int                 p_num_vals; // total number of matrix elements returned by bus

  double *p_iptr; // Pointer used for exchanging values with ghost branches. Note that this pointer is pointed to the buffer used for exchanging values with ghost branches. Its contents should be updated whenever there is a change in v, e.g., when the values from vector X are being mapped to the branches

  double *p_ibr; // Array to hold branch currents. 
  double* p_didt; // Array to hold derivatives

  // Line parameters
  double p_R[3][3];
  double p_L[3][3];
  double p_Linv[3][3];
  double p_minusLinvR[3][3];
  double p_C[3][3];
  bool   p_hasResistance;
  bool   p_hasInductance;

  EmtBranch *p_impl;
  
  friend class boost::serialization::access;
  
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & boost::serialization::base_object<gridpack::component::BaseBranchComponent>(*this)
      & p_nparlines
      & p_status
      & p_cktid
      & p_localoffset
      & p_lineR
      & p_lineX
      & p_nvar
      & p_mode
      & p_time
      & p_TSshift
      & p_isghost
      & p_rank
      & p_offset;    
  }  
  
};

/// The type of network used in the examples application
typedef gridpack::network::BaseNetwork<EmtBus, EmtBranch > EmtNetwork;


BOOST_CLASS_EXPORT_KEY(EmtBus)
BOOST_CLASS_EXPORT_KEY(EmtBranch)


#endif
