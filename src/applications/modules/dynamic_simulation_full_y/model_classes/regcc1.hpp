/*
 *     Copyright (c) 2013 Battelle Memorial Institute
 *     Licensed under modified BSD License. A copy of this license can be found
 *     in the LICENSE file in the top level directory of this distribution.
 */
// -------------------------------------------------------------
/**
 * @file   regcc1.hpp
 * @author Shrirang Abhyankar
 * @Added: January 2, 2023
 * 
 * This is a modified version of REGCB1 model.
 * Reference : https://www.powerworld.com/WebHelp/Content/TransientModels_HTML/Machine%20Model%20REGC_C.htm?TocPath=Transient%20Stability%20Add-On%20(TS)%7CTransient%20Models%7CGenerator%7CMachine%7C_____38
 *
 * This model slightly differs from the reference implementation
 * The 4T_step time constant is replaced by user provided Tg time constant
 * The Rup/Rdown limits are set on the state derivative, not the input.
 *
 *
 * REGCC1 has six more parameters in addition to the parameters from REGCB1
 *  - Kip : Proportional gain for inner loop
 *  - Kii : Integral gain for inner loop
 *  - Kppll: Proportional gain for PLL
 *  - Kipll: Integral gain for PLL
 *  - wmax : Max. limit on PLL output
 *  - wmin : Min. limit on PLL output
 *
 *
 * The entry for REGCC1 model in the dyr file should be as follows:
 *
 * BUSNUM 'REGCC1' ID <REGCB1 paramers> Kip Kii Kppll Kipll wmax wmin
 *
 * where BUSNUM is the bus number, ID is the generator id. The ID is followed by
 * all parameters for REGCB1 and then the additional six parameters in the above
 * order.
 **/


#ifndef _regcc1_h_
#define _regcc1_h_

#include "boost/smart_ptr/shared_ptr.hpp"
#include "base_generator_model.hpp"
#include "base_plant_model.hpp"
#include "cblock.hpp"
#include "dblock.hpp"

namespace gridpack {
namespace dynamic_simulation {
class Regcc1Generator : public BaseGeneratorModel
{
  public:
    /**
     * Basic constructor
     */
    Regcc1Generator();

    /**
     * Basic destructor
     */
    virtual ~Regcc1Generator();

    /**
     * Load parameters from DataCollection object into generator model
     * @param data collection of generator parameters from input files
     * @param index of generator on bus
     * TODO: might want to move this functionality to BaseGeneratorModel
     */
    void load(boost::shared_ptr<gridpack::component::DataCollection>
        data, int idx);
   
    /**
     * Initialize generator model before calculation
     * @param mag voltage magnitude
     * @param ang voltage angle
     * @param ts time step 
     */
    void init(double mag, double ang, double ts);

    /**
     * Return contribution to Norton current
     * @return contribution to Norton vector
     */
    gridpack::ComplexType INorton();

    /**
     * Return Norton impedence
     * @return value of Norton impedence
     */
    gridpack::ComplexType NortonImpedence();

    /**
     * Predict new state variables for time step
     * @param t_inc time step increment
     * @param flag initial step if true
     */
    void predictor(double t_inc, bool flag);

    /**
     * Correct state variables for time step
     * @param t_inc time step increment
     * @param flag initial step if true
     */
    void corrector(double t_inc, bool flag);

    /**
     * Predict part calculate current injections
     * @param flag initial step if true
     */
    void predictor_currentInjection(bool flag);

    /**
     * Corrector part calculate current injections
     * @param flag initial step if true
     */
    void corrector_currentInjection(bool flag);

    /**
     * Set voltage on each generator
     */
    void setVoltage(gridpack::ComplexType voltage);
	
	/**
     * Set frequency on each generator, frequency is perunit
     */
    void setFreq(double dFreq);

  // Called by both predictor and corrector to compute the model
  void computeModel(double t_inc, IntegrationStage int_flag,bool flag);
    /**
	* return true if trip generator successfully
	* 
	*/
    bool tripGenerator();
	
	/**
	* return true if modify the generator parameters successfully
	* input controlTyp: 0: GFI mp adjust; 1: GFI mq adjust; 2: GFI Pset adjust; 3: GFI Qset adjust; others: invalid 
    * input newParValScaletoOrg:  GFI new parameter scale factor to the very initial parameter value at the begining of dynamic simulation
	* 
	*/
    bool applyGeneratorParAdjustment(int controlType, double newParValScaletoOrg);

    /**
     * Write out generator state
     * @param string buffer that contains output
     * @param bufsize size of string buffer
     * @param signal character string used to determine behavior
     */
    bool serialWrite(char* string, const int bufsize, const char* signal);

    /**
     * return a vector containing any generator values that are being
     * watched
     * @param vals vector of watched values
     */
    void getWatchValues(std::vector<double> &vals);

    /**
     * Set internal state parameter in generator
     * @param name character string corresponding to state variable
     * @param value new value for state parameter
     * @return false if no variable corresponding to name is found
     */
    bool setState(std::string name, double value);

    /**
     * Get internal state parameter in generator
     * @param name character string corresponding to state variable
     * @param value current value for state parameter
     * @return false if no variable corresponding to name is found
     */
    bool getState(std::string name, double *value);
		
  private:

    double p_sbase;
    double p_mbase;
    double p_pg, p_qg;
    int p_status;
    bool p_tripped;

    // parameters
    int RateFlag, PQFlag;
    double Tg, Rrpwr, Tfltr, Iqrmax, Iqrmin, Te,Imax;
  double Kip, Kii, Kppll, Kipll, wmax, wmin;
    gridpack::ComplexType Zsource;

    // Blocks
    Filter Ip_blk;
    double Ip; // Output of Ip_blk

    Filter Iq_blk;
    double Iq; // Output of Iq_blk

    Filter Vt_filter_blk;
    double Vt_filter; // Output of Vt_filter blk

  Filter Eq_blk;
  double Eq;

  Filter Ed_blk;
  double Ed;

  PIControl Iq_pi_blk;
  PIControl Ip_pi_blk;

  PIControl Omega_PLL_blk;
  double delta;
  
  Integrator Angle_blk;
  
    double  Ipout,Iqout; // inverter current output in inverter reference frame
    double  Irout, Iiout; // inverter current output in network reference frame
  
    double Ipcmd, Iqcmd, busfreq;  // busfreq is perunit, 1.0
	
    boost::shared_ptr<BaseExciterModel> p_exciter;
    boost::shared_ptr<BasePlantControllerModel> p_plant;

    boost::shared_ptr<BaseMechanicalModel> p_torquecontroller;

    boost::shared_ptr<BaseMechanicalModel> p_pitchcontroller;
    boost::shared_ptr<BaseMechanicalModel> p_drivetrainmodel;

    boost::shared_ptr<BaseMechanicalModel> p_aerodynamicmodel;
    
    gridpack::ComplexType p_INorton;
    gridpack::ComplexType p_Norton_Ya;
  
    double Vt, Vq,theta, VR, VI;

    // Internal variables used in printing
    double Pref; // Plant/Torque controller reference power
    double Taero; // Aerodynamic torque
    double Thetapitch; // Pitch angle (degrees)
    double domega_g;    // Drive train gen. speed deviation (pu)
    double omega_ref; // Reference speed from torque controller (pu)

    std::string p_gen_id;
    int p_bus_num;

    friend class boost::serialization::access;

    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
      {
        ar &
          & p_sbase
	  & p_mbase
          & p_pg & p_qg
          & p_status
          & p_INorton
          & p_bus_num;
      }

};
}  // dynamic_simulation
}  // gridpack
#endif
