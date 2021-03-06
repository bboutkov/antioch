//-----------------------------------------------------------------------bl-
//--------------------------------------------------------------------------
//
// Antioch - A Gas Dynamics Thermochemistry Library
//
// Copyright (C) 2014-2016 Paul T. Bauman, Benjamin S. Kirk,
//                         Sylvain Plessis, Roy H. Stonger
//
// Copyright (C) 2013 The PECOS Development Team
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the Version 2.1 GNU Lesser General
// Public License as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc. 51 Franklin Street, Fifth Floor,
// Boston, MA  02110-1301  USA
//
//-----------------------------------------------------------------------el-

#include "antioch_config.h"

#ifdef ANTIOCH_HAVE_CPPUNIT

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

// C++
#include <limits>

// Anitoch testing
#include "testing_utils.h"
#include "nasa_test_helper.h"

// Antioch
#include "antioch/vector_utils_decl.h"

#include "antioch/chemical_mixture.h"
#include "antioch/stat_mech_thermo.h"
#include "antioch/ideal_gas_micro_thermo.h"
#include "antioch/nasa_mixture.h"
#include "antioch/nasa_evaluator.h"
#include "antioch/nasa7_curve_fit.h"
#include "antioch/nasa9_curve_fit.h"
#include "antioch/nasa_mixture_parsing.h"

#include "antioch/vector_utils.h"

namespace AntiochTesting
{
  template<typename Scalar,typename ThermoSubclass>
  class MacroMicroThermoTestBase : public TestingUtilities<Scalar>,
                                   public CppUnit::TestCase
  {
  public:

    void test_cv_trans()
    {
      for( unsigned int s = 0; s < this->_n_species; s++ )
        this->test_scalar_rel( Scalar(1.5)*this->_gas_consts[s],
                               _thermo->cv_trans(s),
                               std::numeric_limits<Scalar>::epsilon()*10 );
    }

    void test_cv_trans_over_R()
    {
      for( unsigned int s = 0; s < this->_n_species; s++ )
        this->test_scalar_rel( Scalar(1.5),
                               _thermo->cv_trans_over_R(s),
                               std::numeric_limits<Scalar>::epsilon()*10 );
    }

    void test_cv_tr()
    {
      for( unsigned int s = 0; s < this->_n_species; s++ )
        this->test_scalar_rel( this->_n_tr_dofs[s]*this->_gas_consts[s],
                               _thermo->cv_tr(s),
                               std::numeric_limits<Scalar>::epsilon()*10 );
    }

    void test_cv_tr_over_R()
    {
      for( unsigned int s = 0; s < this->_n_species; s++ )
        this->test_scalar_rel( this->_n_tr_dofs[s],
                               _thermo->cv_tr_over_R(s),
                               std::numeric_limits<Scalar>::epsilon()*10 );
    }

    void test_cv_rot()
    {
      for( unsigned int s = 0; s < this->_n_species; s++ )
        this->test_scalar_rel( (this->_n_tr_dofs[s]-Scalar(1.5))*this->_gas_consts[s],
                               _thermo->cv_rot(s),
                               std::numeric_limits<Scalar>::epsilon()*10 );
    }

    void test_cv_rot_over_R()
    {
      for( unsigned int s = 0; s < this->_n_species; s++ )
        this->test_scalar_rel( this->_n_tr_dofs[s]-Scalar(1.5),
                               _thermo->cv_rot_over_R(s),
                               std::numeric_limits<Scalar>::epsilon()*10 );
    }

    void test_cv_tr_mix()
    {
      Scalar cv_exact(0.0);

      for( unsigned int s = 0; s < this->_n_species; s++ )
        cv_exact += this->_n_tr_dofs[s]*this->_gas_consts[s]*this->_mass_fractions[s];

      this->test_scalar_rel( cv_exact,
                             _thermo->cv_tr(this->_mass_fractions),
                             std::numeric_limits<Scalar>::epsilon()*10 );
    }

  protected:

    void init()
    {
      _n_species = 5;

      _species_name_list.reserve(_n_species);
      _species_name_list.push_back( "N2" );
      _species_name_list.push_back( "O2" );
      _species_name_list.push_back( "N" );
      _species_name_list.push_back( "O" );
      _species_name_list.push_back( "NO" );

      _molar_mass.resize(5);
      _molar_mass[2]  = 14.008e-3L; //in SI kg/mol
      _molar_mass[3]  = 16e-3L;     //in SI kg/mol
      _molar_mass[0] = 2.L * _molar_mass[2]; //in SI kg/mol
      _molar_mass[1] = 2.L * _molar_mass[3]; //in SI kg/mol
      _molar_mass[4] = _molar_mass[3] + _molar_mass[2]; //in SI kg/mol

      _gas_consts.resize(5);
      _gas_consts[0] = Antioch::Constants::R_universal<Scalar>() / _molar_mass[0];
      _gas_consts[1] = Antioch::Constants::R_universal<Scalar>() / _molar_mass[1];
      _gas_consts[2] = Antioch::Constants::R_universal<Scalar>() / _molar_mass[2];
      _gas_consts[3] = Antioch::Constants::R_universal<Scalar>() / _molar_mass[3];
      _gas_consts[4] = Antioch::Constants::R_universal<Scalar>() / _molar_mass[4];

      _n_tr_dofs.resize(5);
      _n_tr_dofs[0] = 2.5L;
      _n_tr_dofs[1] = 2.5L;
      _n_tr_dofs[2] = 1.5L;
      _n_tr_dofs[3] = 1.5L;
      _n_tr_dofs[4] = 2.5L;

      _chem_mixture = new Antioch::ChemicalMixture<Scalar>( _species_name_list );

      // Mass fractions
      _mass_fractions.resize( _n_species, 0.2 );
      _mass_fractions[0] = 0.5L;
      _mass_fractions[1] = 0.2L;
      _mass_fractions[2] = 0.1L;
      _mass_fractions[3] = 0.1L;
      _mass_fractions[4] = 0.1L;
    }

    void clear()
    {
      delete _chem_mixture;
    }

    unsigned int _n_species;

    std::vector<std::string> _species_name_list;

    std::vector<Scalar> _molar_mass;
    std::vector<Scalar> _gas_consts;
    std::vector<Scalar> _n_tr_dofs;

    std::vector<Scalar> _mass_fractions;

    Antioch::ChemicalMixture<Scalar> * _chem_mixture;

    Antioch::MacroMicroThermoBase<Scalar,ThermoSubclass> * _thermo;
  };


  template<typename Scalar>
  class StatMechThermoTestBase : public MacroMicroThermoTestBase<Scalar,Antioch::StatMechThermodynamics<Scalar> >
  {
  public:

    void test_cv_vib_over_R()
    {
      // Pick a temperature high enough that should excite the modes
      Scalar T(3141.5);

      for( unsigned int s = 0; s < this->_n_species; s++ )
        {
          Scalar cv_exact(0.0);

          cv_exact = this->eval_cv_vib_over_R_exact_species(s,T);

          this->test_scalar_rel( cv_exact,
                                 this->_thermo->cv_vib_over_R(s,T),
                                 std::numeric_limits<Scalar>::epsilon()*10 );
        }
    }

    void test_cv_vib()
    {
      // Pick a temperature high enough that should excite the modes
      Scalar T(3141.5);

      for( unsigned int s = 0; s < this->_n_species; s++ )
        {
          Scalar cv_exact(0.0);

          cv_exact = this->_gas_consts[s]*this->eval_cv_vib_over_R_exact_species(s,T);

          this->test_scalar_rel( cv_exact,
                                 this->_thermo->cv_vib(s,T),
                                 std::numeric_limits<Scalar>::epsilon()*10 );
        }
    }

    void test_cv_vib_mix()
    {
      // Pick a temperature high enough that should excite the modes
      Scalar T(3141.5);

      Scalar cv_exact(0.0);

      for( unsigned int s = 0; s < this->_n_species; s++ )
        cv_exact += this->_gas_consts[s]*this->eval_cv_vib_over_R_exact_species(s,T)*this->_mass_fractions[s];

      this->test_scalar_rel( cv_exact,
                             this->_thermo->cv_vib(T,this->_mass_fractions),
                             std::numeric_limits<Scalar>::epsilon()*10 );
    }

    void test_cv_el()
    {
      // Pick a temperature high enough that should excite the modes
      Scalar T(13141.5);

      for( unsigned int s = 0; s < this->_n_species; s++ )
        {
          Scalar cv_exact = this->_gas_consts[s]*this->eval_cv_el_over_R_exact_species(s,T);

          this->test_scalar_rel( cv_exact,
                                 this->_thermo->cv_el(s,T),
                                 std::numeric_limits<Scalar>::epsilon()*10 );
        }
    }

    void test_cv_el_mix()
    {
      // Pick a temperature high enough that should excite the modes
      Scalar T(13141.5);

      Scalar cv_exact(0.0);

      for( unsigned int s = 0; s < this->_n_species; s++ )
        cv_exact += this->_gas_consts[s]*this->eval_cv_el_over_R_exact_species(s,T)*this->_mass_fractions[s];

      this->test_scalar_rel( cv_exact,
                             this->_thermo->cv_el(T,this->_mass_fractions),
                             std::numeric_limits<Scalar>::epsilon()*10 );
    }

    void test_T_from_e_tot()
    {
      std::vector<Scalar> T(3);
      T[0] = 300.0L;
      T[1] = 1000.0L;
      T[2] = 6001.0L;

      // We a need a pointer to the derived class since these methods are only
      // in StatMechThermodynamics
      Antioch::StatMechThermodynamics<Scalar> * sm_thermo =
        dynamic_cast<Antioch::StatMechThermodynamics<Scalar> *>( this->_thermo );

      CPPUNIT_ASSERT(sm_thermo);

      for( typename std::vector<Scalar>::const_iterator T_it = T.begin(); T_it < T.end(); ++T_it )
        {
          const Scalar e_tot = sm_thermo->e_tot(*T_it, this->_mass_fractions);

          const Scalar T_test = sm_thermo->T_from_e_tot(e_tot, this->_mass_fractions);

          this->test_scalar_rel( *T_it, T_test, std::numeric_limits<Scalar>::epsilon()*10 );
        }
    }

    void setUp()
    {
      this->init();
      this->_thermo = new Antioch::StatMechThermodynamics<Scalar>( *(this->_chem_mixture) );
      this->init_vib_data();
      this->init_elec_data();
    }

    void tearDown()
    {
      this->clear();
      delete this->_thermo;
    }

  private:

    std::map<std::string,std::vector<Scalar> > _theta_v;
    std::map<std::string,std::vector<unsigned int> > _vib_degeneracy;

    std::map<std::string,std::vector<Scalar> > _theta_el;
    std::map<std::string,std::vector<unsigned int> > _el_degeneracy;

    void init_vib_data()
    {
      // N2
      {
        std::vector<Scalar> & N2_theta = this->_theta_v["N2"];
        std::vector<unsigned int> & N2_dg = this->_vib_degeneracy["N2"];
        N2_theta.resize(1);
        N2_dg.resize(1);

        N2_theta[0] = 3.39500e+03L;
        N2_dg[0] = 1;
      }

      // O2
      {
        std::vector<Scalar> & O2_theta = this->_theta_v["O2"];
        std::vector<unsigned int> & O2_dg = this->_vib_degeneracy["O2"];
        O2_theta.resize(1);
        O2_dg.resize(1);

        O2_theta[0] = 2.23900e+03L;
        O2_dg[0] = 1;
      }

      // NO
      {
        std::vector<Scalar> & NO_theta = this->_theta_v["NO"];
        std::vector<unsigned int> & NO_dg = this->_vib_degeneracy["NO"];
        NO_theta.resize(1);
        NO_dg.resize(1);

        NO_theta[0] = 2.81700e+03L;
        NO_dg[0] = 1;
      }
    }

    void init_elec_data()
    {
      // N2
      {
        std::vector<Scalar> & N2_theta = this->_theta_el["N2"];
        std::vector<unsigned int> & N2_dg = this->_el_degeneracy["N2"];
        N2_theta.resize(15);
        N2_dg.resize(15);

        N2_theta[0] = 0.0L;
        N2_dg[0] = 1;

        N2_theta[1] = 7.22316e+04L;
        N2_dg[1] = 3;

        N2_theta[2] = 8.57786e+04L;
        N2_dg[2] = 6;

        N2_theta[3] = 8.60503e+04L;
        N2_dg[3] = 6;

        N2_theta[4] = 9.53512e+04L;
        N2_dg[4] = 3;

        N2_theta[5] = 9.80564e+04L;
        N2_dg[5] = 1;

        N2_theta[6] = 9.96827e+04L;
        N2_dg[6] = 2;

        N2_theta[7] = 1.04898e+05L;
        N2_dg[7] = 2;

        N2_theta[8] = 1.11649e+05L;
        N2_dg[8] = 5;

        N2_theta[9] = 1.22584e+05L;
        N2_dg[9] = 1;

        N2_theta[10] = 1.24886e+05L;
        N2_dg[10] = 6;

        N2_theta[11] = 1.28248e+05L;
        N2_dg[11] = 6;

        N2_theta[12] = 1.33806e+05L;
        N2_dg[12] = 10;

        N2_theta[13] = 1.40430e+05;
        N2_dg[13] = 6;

        N2_theta[14] = 1.50496e+05;
        N2_dg[14] = 6;
      }

      // O2
      {
        std::vector<Scalar> & O2_theta = this->_theta_el["O2"];
        std::vector<unsigned int> & O2_dg = this->_el_degeneracy["O2"];
        O2_theta.resize(7);
        O2_dg.resize(7);

        O2_theta[0] = 0.0L;
        O2_dg[0] = 3;

        O2_theta[1] = 1.13916e+04L;
        O2_dg[1] = 2;

        O2_theta[2] = 1.89847e+04L;
        O2_dg[2] = 1;

        O2_theta[3] = 4.75597e+04L;
        O2_dg[3] = 1;

        O2_theta[4] = 4.99124e+04L;
        O2_dg[4] = 6;

        O2_theta[5] = 5.09227e+04L;
        O2_dg[5] = 3;

        O2_theta[6] = 7.18986e+04L;
        O2_dg[6] = 3;
      }

      // NO
      {
        std::vector<Scalar> & NO_theta = this->_theta_el["NO"];
        std::vector<unsigned int> & NO_dg = this->_el_degeneracy["NO"];
        NO_theta.resize(16);
        NO_dg.resize(16);

        NO_theta[0] = 0.0L;
        NO_dg[0] = 4;

        NO_theta[1] = 5.46735e+04L;
        NO_dg[1] = 8;

        NO_theta[2] = 6.31714e+04L;
        NO_dg[2] = 2;

        NO_theta[3] = 6.59945e+04L;
        NO_dg[3] = 4;

        NO_theta[4] = 6.90612e+04L;
        NO_dg[4] = 4;

        NO_theta[5] = 7.05000e+04L;
        NO_dg[5] = 4;

        NO_theta[6] = 7.49106e+04L;
        NO_dg[6] = 4;

        NO_theta[7] = 7.62888e+04L;
        NO_dg[7] = 2;

        NO_theta[8] = 8.67619e+04L;
        NO_dg[8] = 4;

        NO_theta[9] = 8.71443e+04L;
        NO_dg[9] = 2;

        NO_theta[10] = 8.88608e+04L;
        NO_dg[10] = 4;

        NO_theta[11] = 8.98176e+04L;
        NO_dg[11] = 4;

        NO_theta[12] = 8.98845e+04L;
        NO_dg[12] = 2;

        NO_theta[13] = 9.04270e+04L;
        NO_dg[13] = 2;

        NO_theta[14] = 9.06428e+04L;
        NO_dg[14] = 2;

        NO_theta[15] = 9.11176e+04L;
        NO_dg[15] = 4;
      }

      // N
      {
        std::vector<Scalar> & N_theta = this->_theta_el["N"];
        std::vector<unsigned int> & N_dg = this->_el_degeneracy["N"];
        N_theta.resize(3);
        N_dg.resize(3);

        N_theta[0] = 0.0L;
        N_dg[0] = 4;

        N_theta[1] = 2.76647e+04L;
        N_dg[1] = 10;

        N_theta[2] = 4.14931e+04L;
        N_dg[2] = 6;
      }

      // O
      {
        std::vector<Scalar> & O_theta = this->_theta_el["O"];
        std::vector<unsigned int> & O_dg = this->_el_degeneracy["O"];
        O_theta.resize(5);
        O_dg.resize(5);

        O_theta[0] = 0.0L;
        O_dg[0] = 5;

        O_theta[1] = 2.27708e+02L;
        O_dg[1] = 3;

        O_theta[2] = 3.26569e+02L;
        O_dg[2] = 1;

        O_theta[3] = 2.28303e+04L;
        O_dg[3] = 5;

        O_theta[4] = 4.86199e+04L;
        O_dg[4] = 1;
      }
    }

    Scalar eval_cv_vib_over_R_exact( Scalar T, Scalar theta )
    {
      // Symbolic expressions from Matlab:
      // e = theta/(exp(theta/T)-1)
      // cv = diff(e,T)
      // gives
      // cv = (theta^2*exp(theta/T))/(T^2*(exp(theta/T) - 1)^2)
      return ((theta*theta)*std::exp(theta/T))/((T*T)*std::pow((exp(theta/T)-Scalar(1.0)),2));
    }

    Scalar eval_cv_vib_over_R_exact_species( unsigned int species,
                                             Scalar T )
    {
      Scalar cv_exact(0.0);

      // We only have vibrational contributions if we have a a diatomic (or bigger) molecule
      if( this->_n_tr_dofs[species] > Scalar(2.0) )
        {
          CPPUNIT_ASSERT( this->_theta_v.find(this->_species_name_list[species]) != this->_theta_v.end() );
          CPPUNIT_ASSERT( this->_vib_degeneracy.find(this->_species_name_list[species]) !=
                          this->_vib_degeneracy.end() );

          const std::vector<Scalar> & theta_v =
            this->_theta_v.find(this->_species_name_list[species])->second;

          const std::vector<unsigned int> & degeneracy =
            this->_vib_degeneracy.find(this->_species_name_list[species])->second;

          CPPUNIT_ASSERT( theta_v.size() == degeneracy.size() );

          for( unsigned int level = 0; level < theta_v.size(); level++ )
            {
              CPPUNIT_ASSERT(level < degeneracy.size() );

              cv_exact +=
                Scalar(degeneracy[level])*this->eval_cv_vib_over_R_exact(T,theta_v[level]);
            }
        }

      return cv_exact;
    }

    Scalar eval_cv_el_over_R_exact( Scalar T, const std::vector<Scalar> & theta_e,
                                    const std::vector<unsigned int> & degeneracy )
    {
      CPPUNIT_ASSERT( theta_e.size() == degeneracy.size() );

      // Expression involves a lot of exp(-theta/T) and it's derivatives w.r.t. T
      // f = exp(-theta/T)
      // diff(f,T) = (theta*exp(-theta/T))/T^2
      //
      // numerator = \sum_{l=levels} \theta_l * g_l * exp(-theta_l/T)
      //
      // denominator = \sum_{l=levels} g_l * exp(-theta_l/T)
      //
      // e/R = numerator/denominator
      //
      // cv = dnum_dT*denominator - numerator/denominator^2*dden_dT
      //
      // dnum_dT = \sum_{l=levels} \theta_l * g_l * theta_l*exp(-theta_l/T)/T^2
      //
      // dden_dT = \sum_{l=levels} g_l * theta_l*exp(-theta_l/T)/T^2

      Scalar num(0.0), denom(0.0), dnum_dT(0.0), ddenom_dT(0.0);
      for( unsigned int levels = 0; levels < theta_e.size(); levels++ )
        {
          Scalar theta(theta_e[levels]);
          Scalar g(degeneracy[levels]);

          num += theta*g*std::exp(-theta/T);
          dnum_dT += theta*g*theta*std::exp(-theta/T)/(T*T);

          denom += g*std::exp(-theta/T);
          ddenom_dT += g*theta*std::exp(-theta/T)/(T*T);
        }

      return dnum_dT/denom - num/(denom*denom)*ddenom_dT;
    }

    Scalar eval_cv_el_over_R_exact_species( unsigned int species,
                                            Scalar T )
    {
      CPPUNIT_ASSERT( this->_theta_el.find(this->_species_name_list[species]) != this->_theta_el.end() );
      CPPUNIT_ASSERT( this->_el_degeneracy.find(this->_species_name_list[species]) !=
                      this->_el_degeneracy.end() );

      const std::vector<Scalar> & theta_el =
        this->_theta_el.find(this->_species_name_list[species])->second;

      const std::vector<unsigned int> & degeneracy =
        this->_el_degeneracy.find(this->_species_name_list[species])->second;

      return this->eval_cv_el_over_R_exact(T, theta_el, degeneracy);
    }

  };

  // These tests can be run at any precision
#define DEFINE_STATMECHTHERMO_SCALAR_TEST_ALL(classname,scalar) \
  class classname : public StatMechThermoTestBase<scalar>       \
  {                                                             \
  public:                                                       \
    CPPUNIT_TEST_SUITE( classname );                            \
    CPPUNIT_TEST(test_cv_trans);                                \
    CPPUNIT_TEST(test_cv_trans_over_R);                         \
    CPPUNIT_TEST(test_cv_tr);                                   \
    CPPUNIT_TEST(test_cv_tr_over_R);                            \
    CPPUNIT_TEST(test_cv_rot);                                  \
    CPPUNIT_TEST(test_cv_rot_over_R);                           \
    CPPUNIT_TEST(test_cv_tr_mix);                               \
    CPPUNIT_TEST_SUITE_END();                                   \
  }

  // These tests can't stand up to long double precision
#define DEFINE_STATMECHTHERMO_SCALAR_TEST_NOLONGDOUBLE(classname,scalar) \
  class classname : public StatMechThermoTestBase<scalar>               \
  {                                                                     \
  public:                                                               \
    CPPUNIT_TEST_SUITE( classname );                                    \
    CPPUNIT_TEST(test_cv_vib_over_R);                                   \
    CPPUNIT_TEST(test_cv_vib);                                          \
    CPPUNIT_TEST(test_cv_vib_mix);                                      \
    CPPUNIT_TEST(test_cv_el);                                           \
    CPPUNIT_TEST(test_cv_el_mix);                                       \
    CPPUNIT_TEST(test_T_from_e_tot);                                    \
    CPPUNIT_TEST_SUITE_END();                                           \
  }

  DEFINE_STATMECHTHERMO_SCALAR_TEST_ALL(StatMechThermoTestFloat,float);
  DEFINE_STATMECHTHERMO_SCALAR_TEST_ALL(StatMechThermoTestDouble,double);
  DEFINE_STATMECHTHERMO_SCALAR_TEST_ALL(StatMechThermoTestLongDouble,long double);

  DEFINE_STATMECHTHERMO_SCALAR_TEST_NOLONGDOUBLE(StatMechThermoTestFloatNOLD,float);
  DEFINE_STATMECHTHERMO_SCALAR_TEST_NOLONGDOUBLE(StatMechThermoTestDoubleNOLD,double);


  CPPUNIT_TEST_SUITE_REGISTRATION( StatMechThermoTestFloat );
  CPPUNIT_TEST_SUITE_REGISTRATION( StatMechThermoTestDouble );
  CPPUNIT_TEST_SUITE_REGISTRATION( StatMechThermoTestLongDouble );
  CPPUNIT_TEST_SUITE_REGISTRATION( StatMechThermoTestFloatNOLD );
  CPPUNIT_TEST_SUITE_REGISTRATION( StatMechThermoTestDoubleNOLD );



  template<typename Scalar, typename NASACurveFit>
  class IdealGasMicroThermoTestBase :
    public MacroMicroThermoTestBase<Scalar,Antioch::IdealGasMicroThermo<Antioch::NASAEvaluator<Scalar,NASACurveFit>,Scalar> >
  {
  public:

    void test_cv_vib()
    {
      for( unsigned int s = 0; s < this->_n_species; s++ )
        {
          // Test first temp interval
          {
            Scalar T(501.2);

            Scalar cv_vib_exact = this->eval_cv_vib_exact(s,T);

            this->test_scalar_rel( cv_vib_exact,
                                   this->_thermo->cv_vib(s,T),
                                   std::numeric_limits<Scalar>::epsilon()*50 );
          }

          // Test second temp interval
          {
            Scalar T(1501.2);

            Scalar cv_vib_exact = this->eval_cv_vib_exact(s,T);

            this->test_scalar_rel( cv_vib_exact,
                                   this->_thermo->cv_vib(s,T),
                                   std::numeric_limits<Scalar>::epsilon()*50 );
          }
        }
    }

    void test_cv_vib_mix()
    {
      // Test first temp interval
      {
        Scalar T(501.2);
        Scalar cv_vib_exact = 0.0;

        for( unsigned int s = 0; s < this->_n_species; s++ )
          cv_vib_exact += this->eval_cv_vib_exact(s,T)*this->_mass_fractions[s];

        this->test_scalar_rel( cv_vib_exact,
                               this->_thermo->cv_vib(T,this->_mass_fractions),
                               std::numeric_limits<Scalar>::epsilon()*50 );
      }

      // Test second temp interval
      {
        Scalar T(1501.2);

        Scalar cv_vib_exact = 0.0;

        for( unsigned int s = 0; s < this->_n_species; s++ )
          cv_vib_exact += this->eval_cv_vib_exact(s,T)*this->_mass_fractions[s];

        this->test_scalar_rel( cv_vib_exact,
                               this->_thermo->cv_vib(T,this->_mass_fractions),
                               std::numeric_limits<Scalar>::epsilon()*50 );
      }
    }

    void test_cv_el()
    {
      Scalar T(1501.2);

      // For IdealGasMicroThermo, none of the electronic modes are populated by assumption.
      Scalar cv_el_exact(0.0);

      for( unsigned int s = 0; s < this->_n_species; s++ )
        this->test_scalar_rel( cv_el_exact,
                               this->_thermo->cv_el(s,T),
                               std::numeric_limits<Scalar>::epsilon() );
    }

    void test_cv_el_mix()
    {
      Scalar T(1501.2);

      // For IdealGasMicroThermo, none of the electronic modes are populated by assumption.
      Scalar cv_el_exact(0.0);

      this->test_scalar_rel( cv_el_exact,
                             this->_thermo->cv_el(T,this->_mass_fractions),
                             std::numeric_limits<Scalar>::epsilon() );
    }

    void setUp()
    {
      this->init();
      this->init_nasa_coeffs();

      _nasa_mixture = new Antioch::NASAThermoMixture<Scalar,NASACurveFit>( *(this->_chem_mixture) );

      this->parse_nasa_coeffs( *(this->_nasa_mixture) );


      _nasa_evaluator = new Antioch::NASAEvaluator<Scalar,NASACurveFit>( *(_nasa_mixture) );

      this->_thermo = new Antioch::IdealGasMicroThermo<Antioch::NASAEvaluator<Scalar,NASACurveFit>,Scalar>
        ( *(_nasa_evaluator), *(this->_chem_mixture) );
    }

    void tearDown()
    {
      delete this->_thermo;
      delete _nasa_evaluator;
      delete _nasa_mixture;
      this->clear();
    }

  protected:

    Antioch::NASAThermoMixture<Scalar,NASACurveFit> * _nasa_mixture;

    Antioch::NASAEvaluator<Scalar,NASACurveFit> * _nasa_evaluator;

    std::map<std::string,std::vector<Scalar> > _nasa_coeffs_lower_interval;
    std::map<std::string,std::vector<Scalar> > _nasa_coeffs_upper_interval;

    Scalar eval_cv_vib_exact( unsigned int species, Scalar T )
    {
      Scalar cv_vib_exact(0.0);

      // cv_vib should only be non-zero if there are rot modes
      if( this->_n_tr_dofs[species] > Scalar(1.5) )
        {
          Scalar cp_over_R = this->eval_cp_exact(species,T);

          // Evaluate cv/R exact
          Scalar cv_over_R = cp_over_R - Scalar(1.0);

          cv_vib_exact = (cv_over_R - this->_n_tr_dofs[species])*this->_gas_consts[species];
        }

      return cv_vib_exact;
    }

    const std::vector<Scalar> & get_nasa_coeffs(  unsigned int species, Scalar T )
    {
      CPPUNIT_ASSERT( _nasa_coeffs_lower_interval.find(this->_species_name_list[species]) !=
                      _nasa_coeffs_lower_interval.end() );

      CPPUNIT_ASSERT( _nasa_coeffs_upper_interval.find(this->_species_name_list[species]) !=
                      _nasa_coeffs_upper_interval.end() );

      if( T < Scalar(1000) )
        return _nasa_coeffs_lower_interval.find(this->_species_name_list[species])->second;
      else
        return _nasa_coeffs_upper_interval.find(this->_species_name_list[species])->second;
    }
    virtual Scalar eval_cp_exact( unsigned int species, Scalar T ) =0;

    virtual void parse_nasa_coeffs( Antioch::NASAThermoMixture<Scalar,NASACurveFit> & nasa_mixture ) =0;
    virtual void init_nasa_coeffs() =0;

  };

  template<typename Scalar>
  class IdealGasMicroThermoTestNASA7Base :
    public IdealGasMicroThermoTestBase<Scalar,Antioch::NASA7CurveFit<Scalar> >,
    public NASA7ThermoTestHelper<Scalar>
  {
  protected:

    virtual void init_nasa_coeffs()
    {
      // We only need to populate N2, O2, and NO for these tests
      // Populate "lower temp interval first
      {
        // N2 coeffs for 300K-1000K
        std::vector<Scalar> & N2_coeffs = this->_nasa_coeffs_lower_interval["N2"];
        N2_coeffs.resize(7);
        N2_coeffs[0] =  3.298677000E+00L;
        N2_coeffs[1] =  1.408240400E-03L;
        N2_coeffs[2] = -3.963222000E-06L;
        N2_coeffs[3] =  5.641515000E-09L;
        N2_coeffs[4] = -2.444854000E-12L;
        N2_coeffs[5] = -1.020899900E+03L;
        N2_coeffs[6] =  3.950372000E+00L;

        // O2 coeffs for 200K-1000K
        std::vector<Scalar> & O2_coeffs = this->_nasa_coeffs_lower_interval["O2"];
        O2_coeffs.resize(7);
        O2_coeffs[0] =  3.782456360E+00L;
        O2_coeffs[1] = -2.996734160E-03L;
        O2_coeffs[2] =  9.847302010E-06L;
        O2_coeffs[3] = -9.681295090E-09L;
        O2_coeffs[4] =  3.243728370E-12L;
        O2_coeffs[5] = -1.063943560E+03L;
        O2_coeffs[6] =  3.657675730E+00L;

        // NO coeffs for 200K-1000K
        std::vector<Scalar> & NO_coeffs = this->_nasa_coeffs_lower_interval["NO"];
        NO_coeffs.resize(7);
        NO_coeffs[0] =  4.218476300E+00L;
        NO_coeffs[1] = -4.638976000E-03L;
        NO_coeffs[2] =  1.104102200E-05L;
        NO_coeffs[3] = -9.336135400E-09L;
        NO_coeffs[4] =  2.803577000E-12L;
        NO_coeffs[5] =  9.844623000E+03L;
        NO_coeffs[6] =  2.280846400E+00L;
      }

      // Now populate "upper" temp interval first
      {
        // N2 coeffs for 1000K-5000K
        std::vector<Scalar> & N2_coeffs = this->_nasa_coeffs_upper_interval["N2"];
        N2_coeffs.resize(7);
        N2_coeffs[0] =  2.926640000E+00L;
        N2_coeffs[1] =  1.487976800E-03L;
        N2_coeffs[2] = -5.684760000E-07L;
        N2_coeffs[3] =  1.009703800E-10L;
        N2_coeffs[4] = -6.753351000E-15L;
        N2_coeffs[5] = -9.227977000E+02L;
        N2_coeffs[6] =  5.980528000E+00L;

        // O2 coeffs for 1000K-3500K
        std::vector<Scalar> & O2_coeffs = this->_nasa_coeffs_upper_interval["O2"];
        O2_coeffs.resize(7);
        O2_coeffs[0] =  3.282537840E+00L;
        O2_coeffs[1] =  1.483087540E-03L;
        O2_coeffs[2] = -7.579666690E-07L;
        O2_coeffs[3] =  2.094705550E-10L;
        O2_coeffs[4] = -2.167177940E-14L;
        O2_coeffs[5] = -1.088457720E+03L;
        O2_coeffs[6] =  5.453231290E+00L;

        // NO coeffs for 1000K-6000K
        std::vector<Scalar> & NO_coeffs = this->_nasa_coeffs_upper_interval["NO"];
        NO_coeffs.resize(7);
        NO_coeffs[0] =  3.260605600E+00L;
        NO_coeffs[1] =  1.191104300E-03L;
        NO_coeffs[2] = -4.291704800E-07L;
        NO_coeffs[3] =  6.945766900E-11L;
        NO_coeffs[4] = -4.033609900E-15L;
        NO_coeffs[5] =  9.920974600E+03L;
        NO_coeffs[6] =  6.369302700E+00L;
      }

    }

    virtual void parse_nasa_coeffs( Antioch::NASAThermoMixture<Scalar,Antioch::NASA7CurveFit<Scalar> > & nasa_mixture )
    {
      std::string thermo_filename =
        std::string(ANTIOCH_SHARE_XML_INPUT_FILES_SOURCE_PATH)+"gri30.xml";

       Antioch::read_nasa_mixture_data( nasa_mixture, thermo_filename, Antioch::XML );
    }

    virtual Scalar eval_cp_exact( unsigned int species, Scalar T )
    {
      const std::vector<Scalar> & coeffs = this->get_nasa_coeffs(species,T);

      CPPUNIT_ASSERT( coeffs.size() == 7 );

      return this->cp_exact( T, coeffs[0], coeffs[1], coeffs[2], coeffs[3], coeffs[4] );
    }

  };

  template<typename Scalar>
  class IdealGasMicroThermoTestNASA9Base :
    public IdealGasMicroThermoTestBase<Scalar,Antioch::NASA9CurveFit<Scalar> >,
    public NASA9ThermoTestHelper<Scalar>
  {
  protected:

    virtual void init_nasa_coeffs()
    {
      // We only need to populate N2, O2, and NO for these tests
      // Populate "lower temp interval first
      {
        // N2 coeffs for 200K-1000K
        std::vector<Scalar> & N2_coeffs = this->_nasa_coeffs_lower_interval["N2"];
        N2_coeffs.resize(9);
        N2_coeffs[0] =  2.21037122e+04L;
        N2_coeffs[1] = -3.81846145e+02L;
        N2_coeffs[2] =  6.08273815e+00L;
        N2_coeffs[3] = -8.53091381e-03L;
        N2_coeffs[4] =  1.38464610e-05L;
        N2_coeffs[5] = -9.62579293e-09L;
        N2_coeffs[6] =  2.51970560e-12L;
        N2_coeffs[7] =  7.10845911e+02L;
        N2_coeffs[8] = -1.07600320e+01L;

        // O2 coeffs for 200K-1000K
        std::vector<Scalar> & O2_coeffs = this->_nasa_coeffs_lower_interval["O2"];
        O2_coeffs.resize(9);
        O2_coeffs[0] = -3.42556269e+04L;
        O2_coeffs[1] =  4.84699986e+02L;
        O2_coeffs[2] =  1.11901159e+00L;
        O2_coeffs[3] =  4.29388743e-03L;
        O2_coeffs[4] = -6.83627313e-07L;
        O2_coeffs[5] = -2.02337478e-09L;
        O2_coeffs[6] =  1.03904064e-12L;
        O2_coeffs[7] = -3.39145434e+03L;
        O2_coeffs[8] =  1.84969912e+01L;

        // NO coeffs for 200K-1000K
        std::vector<Scalar> & NO_coeffs = this->_nasa_coeffs_lower_interval["NO"];
        NO_coeffs.resize(9);
        NO_coeffs[0] = -1.14391658e+04L;
        NO_coeffs[1] =  1.53646774e+02L;
        NO_coeffs[2] =  3.43146865e+00L;
        NO_coeffs[3] = -2.66859213e-03L;
        NO_coeffs[4] =  8.48139877e-06L;
        NO_coeffs[5] = -7.68511079e-09L;
        NO_coeffs[6] =  2.38679758e-12L;
        NO_coeffs[7] =  9.09794974e+03L;
        NO_coeffs[8] =  6.72872795e+00L;
      }

      // Now populate "upper" temp interval first
      {
        // N2 coeffs for 1000K-6000K
        std::vector<Scalar> & N2_coeffs = this->_nasa_coeffs_upper_interval["N2"];
        N2_coeffs.resize(9);
        N2_coeffs[0] =  5.87709908e+05L;
        N2_coeffs[1] = -2.23924255e+03L;
        N2_coeffs[2] =  6.06694267e+00L;
        N2_coeffs[3] = -6.13965296e-04L;
        N2_coeffs[4] =  1.49179819e-07L;
        N2_coeffs[5] = -1.92309442e-11L;
        N2_coeffs[6] =  1.06194871e-15L;
        N2_coeffs[7] =  1.28320618e+04L;
        N2_coeffs[8] = -1.58663484e+01L;

        // O2 coeffs for 1000K-6000K
        std::vector<Scalar> & O2_coeffs = this->_nasa_coeffs_upper_interval["O2"];
        O2_coeffs.resize(9);
        O2_coeffs[0] = -1.03793994e+06L;
        O2_coeffs[1] =  2.34483275e+03L;
        O2_coeffs[2] =  1.81972949e+00L;
        O2_coeffs[3] =  1.26784887e-03L;
        O2_coeffs[4] = -2.18807142e-07L;
        O2_coeffs[5] =  2.05372411e-11L;
        O2_coeffs[6] = -8.19349062e-16L;
        O2_coeffs[7] = -1.68901253e+04L;
        O2_coeffs[8] =  1.73871835e+01L;

        // NO coeffs for 1000K-6000K
        std::vector<Scalar> & NO_coeffs = this->_nasa_coeffs_upper_interval["NO"];
        NO_coeffs.resize(9);
        NO_coeffs[0] =  2.23903708e+05L;
        NO_coeffs[1] = -1.28965624e+03L;
        NO_coeffs[2] =  5.43394039e+00L;
        NO_coeffs[3] = -3.65605546e-04L;
        NO_coeffs[4] =  9.88101763e-08L;
        NO_coeffs[5] = -1.41608327e-11L;
        NO_coeffs[6] =  9.38021642e-16L;
        NO_coeffs[7] =  1.75029422e+04L;
        NO_coeffs[8] = -8.50169908e+00L;
      }
    }

    virtual Scalar eval_cp_exact( unsigned int species, Scalar T )
    {
      const std::vector<Scalar> & coeffs = this->get_nasa_coeffs(species,T);

      CPPUNIT_ASSERT( coeffs.size() == 9 );

      return this->cp_exact( T, coeffs[0], coeffs[1], coeffs[2],
                             coeffs[3], coeffs[4], coeffs[5], coeffs[6] );
    }

    virtual void parse_nasa_coeffs( Antioch::NASAThermoMixture<Scalar,Antioch::NASA9CurveFit<Scalar> > & nasa_mixture )
    {
      std::string thermo_filename =
        std::string(ANTIOCH_SHARE_XML_INPUT_FILES_SOURCE_PATH)+"nasa9_thermo.xml";

      Antioch::read_nasa_mixture_data( nasa_mixture, thermo_filename, Antioch::XML );
    }

  };

#define DEFINE_IDEALGASMICROTHERMO_SCALAR_TEST(Classname,BaseClass,Scalar) \
  class Classname : public BaseClass<Scalar>                            \
  {                                                                     \
  public:                                                               \
    CPPUNIT_TEST_SUITE( Classname );                                    \
    CPPUNIT_TEST(test_cv_trans);                                        \
    CPPUNIT_TEST(test_cv_trans_over_R);                                 \
    CPPUNIT_TEST(test_cv_tr);                                           \
    CPPUNIT_TEST(test_cv_tr_over_R);                                    \
    CPPUNIT_TEST(test_cv_rot);                                          \
    CPPUNIT_TEST(test_cv_rot_over_R);                                   \
    CPPUNIT_TEST(test_cv_tr_mix);                                       \
    CPPUNIT_TEST(test_cv_vib);                                          \
    CPPUNIT_TEST(test_cv_vib_mix);                                      \
    CPPUNIT_TEST(test_cv_el);                                           \
    CPPUNIT_TEST(test_cv_el_mix);                                       \
    CPPUNIT_TEST_SUITE_END();                                           \
  }

  DEFINE_IDEALGASMICROTHERMO_SCALAR_TEST(IdealGasMicroThermoNASA7FloatTest,
                                         IdealGasMicroThermoTestNASA7Base,
                                         float);
  DEFINE_IDEALGASMICROTHERMO_SCALAR_TEST(IdealGasMicroThermoNASA7DoubleTest,
                                         IdealGasMicroThermoTestNASA7Base,
                                         double);
  DEFINE_IDEALGASMICROTHERMO_SCALAR_TEST(IdealGasMicroThermoNASA7LongDoubleTest,
                                         IdealGasMicroThermoTestNASA7Base,
                                         long double);

  DEFINE_IDEALGASMICROTHERMO_SCALAR_TEST(IdealGasMicroThermoNASA9FloatTest,
                                         IdealGasMicroThermoTestNASA9Base,
                                         float);
  DEFINE_IDEALGASMICROTHERMO_SCALAR_TEST(IdealGasMicroThermoNASA9DoubleTest,
                                         IdealGasMicroThermoTestNASA9Base,
                                         double);
  DEFINE_IDEALGASMICROTHERMO_SCALAR_TEST(IdealGasMicroThermoNASA9LongDoubleTest,
                                         IdealGasMicroThermoTestNASA9Base,
                                         long double);

  CPPUNIT_TEST_SUITE_REGISTRATION( IdealGasMicroThermoNASA7FloatTest );
  CPPUNIT_TEST_SUITE_REGISTRATION( IdealGasMicroThermoNASA7DoubleTest );
  CPPUNIT_TEST_SUITE_REGISTRATION( IdealGasMicroThermoNASA7LongDoubleTest );
  CPPUNIT_TEST_SUITE_REGISTRATION( IdealGasMicroThermoNASA9FloatTest );
  CPPUNIT_TEST_SUITE_REGISTRATION( IdealGasMicroThermoNASA9DoubleTest );
  CPPUNIT_TEST_SUITE_REGISTRATION( IdealGasMicroThermoNASA9LongDoubleTest );

} // end namespace AntiochTesting

#endif // ANTIOCH_HAVE_CPPUNIT
