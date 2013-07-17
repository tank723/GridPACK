// -------------------------------------------------------------
/**
 * @file   pf_components.cpp
 * @author Bruce Palmer
 * @date   June 4, 2013
 * 
 * @brief  
 * 
 * 
 */
// -------------------------------------------------------------

#include <vector>

#include "boost/smart_ptr/shared_ptr.hpp"
#include "gridpack/utilities/complex.hpp"
#include "gridpack/component/base_component.hpp"
#include "gridpack/component/data_collection.hpp"
#include "gridpack/applications/dynamic_simulation/dynsim_components.hpp"

/**
 *  Simple constructor
 */
gridpack::dynsim::DynSimBus::DynSimBus(void)
{
  p_shunt_gs = 0.0;
  p_shunt_bs = 0.0;
  p_mode = YBUS;
  setReferenceBus(false);
}

/**
 *  Simple destructor
 */
gridpack::dynsim::DynSimBus::~DynSimBus(void)
{
}

/**
 *  Return size of matrix block contributed by the component
 *  @param isize, jsize: number of rows and columns of matrix block
 *  @return: false if network component does not contribute matrix element
 */
bool gridpack::dynsim::DynSimBus::matrixDiagSize(int *isize, int *jsize) const
{
  if (p_mode == JACOBIAN && getReferenceBus()) {
    *isize = 0;
    *jsize = 0;
    return false;
  } else if (p_mode == JACOBIAN) {
    *isize = 2;
    *jsize = 2;
  } else if (p_mode == GENERATOR) {
  }
  return true;
}

/**
 * Return the values of the matrix block. The values are
 * returned in row-major order
 * @param values: pointer to matrix block values
 * @return: false if network component does not contribute matrix element
 */
bool gridpack::dynsim::DynSimBus::matrixDiagValues(void *values)
{
  if (p_mode == YBUS) {
    gridpack::ComplexType ret(p_ybusr,p_ybusi);
    (static_cast<gridpack::ComplexType*>(values))[0] = p_ybusr;
    (static_cast<gridpack::ComplexType*>(values))[1] = p_ybusi;
    (static_cast<gridpack::ComplexType*>(values))[2] = -p_ybusi;
    (static_cast<gridpack::ComplexType*>(values))[3] = p_ybusr;
    return true;
  } else if (p_mode == JACOBIAN) {
    if (!getReferenceBus()) {
      double branch_values[4];
      // TODO: More stuff here
      std::vector<boost::shared_ptr<BaseComponent> > branches;
      getNeighborBranches(branches);
      int size = branches.size();
      int i;
      (static_cast<ComplexType*>(values))[0] = 0.0;
      (static_cast<ComplexType*>(values))[1] = 0.0;
      (static_cast<ComplexType*>(values))[2] = 2.0*p_v*p_ybusr;
      (static_cast<ComplexType*>(values))[3] = -2.0*p_v*p_ybusi;
      // HACK: Need to cast pointer, is there a better way?
      for (i=0; i<size; i++) {
	(dynamic_cast<gridpack::dynsim::DynSimBranch*>(branches[i].get()))->
          getJacobian(this, branch_values);
        (static_cast<ComplexType*>(values))[0] -= p_v*branch_values[0];
        (static_cast<ComplexType*>(values))[1] += p_v*branch_values[1];
        (static_cast<ComplexType*>(values))[2] += p_v*branch_values[2];
        (static_cast<ComplexType*>(values))[3] += p_v*branch_values[3];
      }
    } else {
      return false;
    }
  } else if (p_mode == GENERATOR) {
  }
}

/**
 * Return the size of the block that this component contributes to the
 * vector
 * @param size: size of vector block
 * @return: false if component does not contribute to vector
 */
bool gridpack::dynsim::DynSimBus::vectorSize(int *size) const
{
  if (p_mode == JACOBIAN && getReferenceBus()) {
    *size = 0;
    return false;
  if (p_mode == JACOBIAN) {
    *size = 2;
  } else if (p_mode == GENERATOR) {
    *size = 2;
  }
  return true;
}

/**
 * Return the values of the vector block
 * @param values: pointer to vector values
 * @return: false if network component does not contribute
 *        vector element
 */
bool gridpack::dynsim::DynSimBus::vectorValues(void *values)
{
  if (p_mode == JACOBIAN) {
    std::vector<boost::shared_ptr<BaseComponent> > branches;
    getNeighborBranches(branches);
    int size = branches.size();
    int i;
    double P, Q, p, q;
    P = 0.0;
    Q = 0.0;
    for (i=0; i<size; i++) {
      boost::shared_ptr<gridpack::dynsim::DynSimBranch>
        branch(dynamic_cast<gridpack::dynsim::DynSimBranch*>(branches[i].get()));
      branch->getPQ(this, &p, &q);
      P += p;
      Q += q;
    }
    P -= p_P0;
    Q -= p_Q0;
    (static_cast<gridpack::ComplexType*>(values))[0] = P;
    (static_cast<gridpack::ComplexType*>(values))[1] = Q;
    return true;
  } else if (p_mode == GENERATOR) {
  }
}

void gridpack::dynsim::DynSimBus::setYBus(void)
{
  gridpack::ComplexType ret(0.0,0.0);
  std::vector<boost::shared_ptr<BaseComponent> > branches;
  getNeighborBranches(branches);
  int size = branches.size();
  int i;
  // HACK: Need to cast pointer, is there a better way?
  for (i=0; i<size; i++) {
    boost::shared_ptr<gridpack::dynsim::DynSimBranch>
      branch(dynamic_cast<gridpack::dynsim::DynSimBranch*>(branches[i].get()));
    ret -= branch->getAdmittance();
    ret += branch->getTransformer(this);
    ret += branch->getShunt(this);
  }
  if (p_shunt) {
    gridpack::ComplexType shunt(p_shunt_gs,p_shunt_bs);
    ret += shunt;
  }
  p_ybusr = real(ret);
  p_ybusi = imag(ret);
}

/**
 * Load values stored in DataCollection object into DynSimBus object. The
 * DataCollection object will have been filled when the network was created
 * from an external configuration file
 * @param data: DataCollection object contain parameters relevant to this
 *       bus that were read in when network was initialized
 */
void gridpack::dynsim::DynSimBus::load(
  const boost::shared_ptr<gridpack::component::DataCollection> &data)
{
  p_shunt = true;
  p_shunt = p_shunt && data->getValue(BUS_SHUNT_GS, &p_shunt_gs);
  p_shunt = p_shunt && data->getValue(BUS_SHUNT_BS, &p_shunt_bs);
  // TODO: Need to get values of P0 and Q0 from Network Configuration file
}

/**
 * Return the value of the voltage magnitude on this bus
 * @return: voltage magnitude
 */
double gridpack::dynsim::DynSimBus::getVoltage()
{
}

/**
 * Return the value of the phase angle on this bus
 * @return: phase angle
 */
double gridpack::dynsim::DynSimBus::getPhase()
{
}

/**
 *  Simple constructor
 */
gridpack::dynsim::DynSimBranch::DynSimBranch(void)
{
  p_reactance = 0.0;
  p_resistance = 0.0;
  p_tap_ratio = 1.0;
  p_phase_shift = 0.0;
  p_charging = 0.0;
  p_shunt_admt_g1 = 0.0;
  p_shunt_admt_b1 = 0.0;
  p_shunt_admt_g2 = 0.0;
  p_shunt_admt_b2 = 0.0;
  p_mode = YBUS;
}

/**
 *  Simple destructor
 */
gridpack::dynsim::DynSimBranch::~DynSimBranch(void)
{
}

/**
 * Return size of off-diagonal matrix block contributed by the component
 * for the forward/reverse directions
 * @param isize, jsize: number of rows and columns of matrix block
 * @return: false if network component does not contribute matrix element
 */
bool gridpack::dynsim::DynSimBranch::matrixForwardSize(int *isize, int *jsize) const
{
  if (p_mode == JACOBIAN) {
    boost::shared_ptr<gridpack::dynsim::DynSimBus>
      bus1(dynamic_cast<gridpack::dynsim::DynSimBus*>(getBus1().get()));
    boost::shared_ptr<gridpack::dynsim::DynSimBus>
      bus2(dynamic_cast<gridpack::dynsim::DynSimBus*>(getBus2().get()));
    bool ok = bus1->getReferenceBus();
    ok = ok & bus2->getReferenceBus();
    if (ok) {
      *isize = 2;
      *jsize = 2;
      return true;
    } else {
      *isize = 0;
      *jsize = 0;
      return false;
    }
  } else if (p_mode == YBUS) {
    *isize = 2;
    *jsize = 2;
    return true;
  } else if (p_mode == GENERATOR) {
  }
}
bool gridpack::dynsim::DynSimBranch::matrixReverseSize(int *isize, int *jsize) const
{
  if (p_mode == JACOBIAN) {
    boost::shared_ptr<gridpack::dynsim::DynSimBus>
      bus1(dynamic_cast<gridpack::dynsim::DynSimBus*>(getBus1().get()));
    boost::shared_ptr<gridpack::dynsim::DynSimBus> 
      bus2(dynamic_cast<gridpack::dynsim::DynSimBus*>(getBus2().get()));
    bool ok = bus1->getReferenceBus();
    ok = ok & bus2->getReferenceBus();
    if (ok) {
      *isize = 2;
      *jsize = 2;
      return true;
    } else {
      *isize = 0;
      *jsize = 0;
      return false;
    }
  } else if (p_mode == YBUS) {
    *isize = 2;
    *jsize = 2;
    return true;
  } else if (p_mode == GENERATOR) {
  }
}

/**
 * Return the values of the off-diagonal matrix block. The values are
 * returned in row-major order
 * @param values: pointer to matrix block values
 * @return: false if network component does not contribute matrix element
 */
bool gridpack::dynsim::DynSimBranch::matrixForwardValues(void *values)
{
  if (p_mode == JACOBIAN) {
    boost::shared_ptr<gridpack::dynsim::DynSimBus>
      bus1(dynamic_cast<gridpack::dynsim::DynSimBus*>(getBus1().get()));
    boost::shared_ptr<gridpack::dynsim::DynSimBus>
      bus2(dynamic_cast<gridpack::dynsim::DynSimBus*>(getBus2().get()));
    bool ok = bus1->getReferenceBus();
    ok = ok & bus2->getReferenceBus();
    if (ok) {
      double t11, t12, t21, t22;
      double cs = cos(p_theta);
      double sn = sin(p_theta);
      (static_cast<gridpack::ComplexType*>(values))[0] = (p_ybusr*sn - p_ybusi*cs);
      (static_cast<gridpack::ComplexType*>(values))[1] = (p_ybusr*cs + p_ybusi*sn);
      (static_cast<gridpack::ComplexType*>(values))[2] = (p_ybusr*cs + p_ybusi*sn);
      (static_cast<gridpack::ComplexType*>(values))[3] = (p_ybusr*sn - p_ybusi*cs);
      (static_cast<gridpack::ComplexType*>(values))[0] *= ((bus1->getVoltage())*(bus2->getVoltage()));
      (static_cast<gridpack::ComplexType*>(values))[1] *= bus1->getVoltage();
      (static_cast<gridpack::ComplexType*>(values))[2] *= -((bus1->getVoltage())*(bus2->getVoltage()));
      (static_cast<gridpack::ComplexType*>(values))[3] *= bus1->getVoltage();
      return true;
    } else {
      return false;
    }
  } else if (p_mode == YBUS) {
    (static_cast<gridpack::ComplexType*>(values))[0] = p_ybusr;
    (static_cast<gridpack::ComplexType*>(values))[1] = p_ybusi;
    (static_cast<gridpack::ComplexType*>(values))[2] = -p_ybusi;
    (static_cast<gridpack::ComplexType*>(values))[3] = p_ybusr;
    return true;
  } else if (p_mode == GENERATOR) {
  }
}
bool gridpack::dynsim::DynSimBranch::matrixReverseValues(void *values)
{
  if (p_mode == JACOBIAN) {
    boost::shared_ptr<gridpack::dynsim::DynSimBus>
      bus1(dynamic_cast<gridpack::dynsim::DynSimBus*>(getBus1().get()));
    boost::shared_ptr<gridpack::dynsim::DynSimBus>
      bus2(dynamic_cast<gridpack::dynsim::DynSimBus*>(getBus2().get()));
    bool ok = bus1->getReferenceBus();
    ok = ok & bus2->getReferenceBus();
    if (ok) {
      double t11, t12, t21, t22;
      double cs = cos(-p_theta);
      double sn = sin(-p_theta);
      (static_cast<gridpack::ComplexType*>(values))[0] = (p_ybusr*sn - p_ybusi*cs);
      (static_cast<gridpack::ComplexType*>(values))[1] = (p_ybusr*cs + p_ybusi*sn);
      (static_cast<gridpack::ComplexType*>(values))[2] = (p_ybusr*cs + p_ybusi*sn);
      (static_cast<gridpack::ComplexType*>(values))[3] = (p_ybusr*sn - p_ybusi*cs);
      (static_cast<gridpack::ComplexType*>(values))[0] *= ((bus1->getVoltage())*(bus2->getVoltage()));
      (static_cast<gridpack::ComplexType*>(values))[1] *= bus2->getVoltage();
      (static_cast<gridpack::ComplexType*>(values))[2] *= -((bus1->getVoltage())*(bus2->getVoltage()));
      (static_cast<gridpack::ComplexType*>(values))[3] *= bus2->getVoltage();
      return true;
    } else {
      return false;
    }
  } else if (p_mode == YBUS) {
    (static_cast<gridpack::ComplexType*>(values))[0] = p_ybusr;
    (static_cast<gridpack::ComplexType*>(values))[1] = p_ybusi;
    (static_cast<gridpack::ComplexType*>(values))[2] = -p_ybusi;
    (static_cast<gridpack::ComplexType*>(values))[3] = p_ybusr;
    return true;
  } else if (p_mode == GENERATOR) {
  }
}

// Calculate contributions to the admittance matrix from the branches
void gridpack::dynsim::DynSimBranch::setYBus(void)
{
  gridpack::ComplexType ret(p_resistance,p_reactance);
  ret = -1.0/ret;
  gridpack::ComplexType a(cos(p_phase_shift),sin(p_phase_shift));
  a = p_tap_ratio*a;
  ret = ret - ret/conj(a);
  p_ybusr = real(ret);
  p_ybusi = imag(ret);
  // Not really a contribution to the admittance matrix but might as well
  // calculate phase angle difference between buses at each end of branch
  boost::shared_ptr<gridpack::dynsim::DynSimBus>
    bus1(dynamic_cast<gridpack::dynsim::DynSimBus*>(getBus1().get()));
  boost::shared_ptr<gridpack::dynsim::DynSimBus>
    bus2(dynamic_cast<gridpack::dynsim::DynSimBus*>(getBus2().get()));
  p_theta = bus1->getPhase() - bus2->getPhase();
   
}

/**
 * Load values stored in DataCollection object into DynSimBranch object. The
 * DataCollection object will have been filled when the network was created
 * from an external configuration file
 * @param data: DataCollection object contain parameters relevant to this
 *       branch that were read in when network was initialized
 */
void gridpack::dynsim::DynSimBranch::load(
  const boost::shared_ptr<gridpack::component::DataCollection> &data)
{
  bool ok = true;
  ok = ok && data->getValue(BRANCH_REACTANCE, &p_reactance);
  ok = ok && data->getValue(BRANCH_RESISTANCE, &p_resistance);
  p_xform = true;
  p_xform = p_xform && data->getValue(BRANCH_TAP_RATIO, &p_tap_ratio);
  p_xform = p_xform && data->getValue(BRANCH_PHASE_SHIFT, &p_phase_shift);
  p_shunt = true;
  p_shunt = p_shunt && data->getValue(BRANCH_CHARGING, &p_charging);
  p_shunt = p_shunt && data->getValue(BRANCH_SHUNT_ADMTTNC_G1, &p_shunt_admt_g1);
  p_shunt = p_shunt && data->getValue(BRANCH_SHUNT_ADMTTNC_B1, &p_shunt_admt_b1);
  p_shunt = p_shunt && data->getValue(BRANCH_SHUNT_ADMTTNC_G2, &p_shunt_admt_g2);
  p_shunt = p_shunt && data->getValue(BRANCH_SHUNT_ADMTTNC_B2, &p_shunt_admt_b2);
}

/**
 * Return the complex admittance of the branch
 * @return: complex addmittance of branch
 */
gridpack::ComplexType gridpack::dynsim::DynSimBranch::getAdmittance(void)
{
  gridpack::ComplexType ret(p_resistance, p_reactance);
  return -1.0/ret;
}

/**
 * Return transformer contribution from the branch to the calling
 * bus
 * @param bus: pointer to the bus making the call
 * @return: contribution to Y matrix from branch
 */
gridpack::ComplexType
gridpack::dynsim::DynSimBranch::getTransformer(gridpack::dynsim::DynSimBus *bus)
{
  if (p_xform) {
    gridpack::ComplexType ret(p_resistance,p_reactance);
    ret = -1.0/ret;
    // HACK: pointer comparison, maybe could handle this better
    if (bus == getBus1().get()) {
      ret = ret/(p_tap_ratio*p_tap_ratio);
    } else if (bus == getBus2().get()) {
      // No further action required
    } else {
      // TODO: Some kind of error
    }
    return ret;
  } else {
    gridpack::ComplexType ret(0.0,0.0);
    return ret;
  }
}

/**
 * Return the contribution to a bus from shunts
 * @param bus: pointer to the bus making the call
 * @return: contribution to Y matrix from shunts associated with branches
 */
gridpack::ComplexType
gridpack::dynsim::DynSimBranch::getShunt(gridpack::dynsim::DynSimBus *bus)
{
  double retr, reti;
  if (p_shunt) {
    retr = 0.5*p_charging;
    reti = 0.0;
    // HACK: pointer comparison, maybe could handle this better
    if (bus == getBus1().get()) {
      retr += p_shunt_admt_g1;
      reti += p_shunt_admt_b1;
    } else if (bus == getBus2().get()) {
      retr += p_shunt_admt_g2;
      reti += p_shunt_admt_b2;
    } else {
      // TODO: Some kind of error
    }
  } else {
    retr = 0.0;
    reti = 0.0;
  }
  return gridpack::ComplexType(retr,reti);
}

/**
 * Return the contribution to the Jacobian for the dynsim equations from
 * a branch
 * @param bus: pointer to the bus making the call
 * @param values: an array of 4 doubles that holds return metrix elements
 */
void gridpack::dynsim::DynSimBranch::getJacobian(gridpack::dynsim::DynSimBus *bus, double *values)
{
  double v;
  double cs, sn;
  if (bus == getBus1().get()) {
    boost::shared_ptr<gridpack::dynsim::DynSimBus>
      bus2(dynamic_cast<gridpack::dynsim::DynSimBus*>(getBus2().get()));
    v = bus2->getVoltage();
    cs = cos(p_theta);
    sn = sin(p_theta);
  } else if (bus == getBus2().get()) {
    boost::shared_ptr<gridpack::dynsim::DynSimBus>
      bus1(dynamic_cast<gridpack::dynsim::DynSimBus*>(getBus1().get()));
    v = bus1->getVoltage();
    cs = cos(-p_theta);
    sn = sin(-p_theta);
  } else {
    // TODO: Some kind of error
  }
  values[0] = v*(p_ybusr*sn - p_ybusi*cs);
  values[1] = v*(p_ybusr*cs + p_ybusi*sn);
  values[2] = (p_ybusr*cs + p_ybusi*sn);
  values[3] = (p_ybusr*sn - p_ybusi*cs);
}

/**
 * Return contribution to constraints
 * @param p: real part of constraint
 * @param q: imaginary part of constraint
 */
void gridpack::dynsim::DynSimBranch::getPQ(gridpack::dynsim::DynSimBus *bus, double *p, double *q)
{
  double cs, sn;
  if (bus == getBus1().get()) {
    cs = cos(p_theta);
    sn = sin(p_theta);
  } else if (bus == getBus2().get()) {
    cs = cos(-p_theta);
    sn = sin(-p_theta);
  } else {
    // TODO: Some kind of error
  }
    boost::shared_ptr<gridpack::dynsim::DynSimBus>
      bus1(dynamic_cast<gridpack::dynsim::DynSimBus*>(getBus1().get()));
    double v1 = bus1->getVoltage();
    boost::shared_ptr<gridpack::dynsim::DynSimBus>
      bus2(dynamic_cast<gridpack::dynsim::DynSimBus*>(getBus2().get()));
    double v2 = bus2->getVoltage();
    *p = v1*v2*(p_ybusr*cs+p_ybusi*sn);
    *q = v1*v2*(p_ybusr*sn-p_ybusi*cs);
}
