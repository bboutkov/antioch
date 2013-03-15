//-----------------------------------------------------------------------bl-
//--------------------------------------------------------------------------
// 
// Antioch - A Gas Dynamics Thermochemistry Library
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
//
// $Id$
//
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

#ifndef ANTIOCH_WILKE_EVALUATOR_H
#define ANTIOCH_WILKE_EVALUATOR_H

// Antioch
#include "antioch/metaprogramming.h"

namespace Antioch
{

  
  template<class Viscosity, class ThermalConductivity, class CoeffType=double>
  class WilkeEvaluator
  {
  public:

    WilkeEvaluator( const WilkeMixture<CoeffType>& mixture,
		    const MixtureViscosity<Viscosity,CoeffType>& viscosity,
		    const ThermalConductivity& conductivity );

    ~WilkeEvaluator();

    template <typename StateType, typename VectorStateType>
    StateType mu( const StateType T,
		  const VectorStateType& mass_fractions ) const;

    template <typename StateType, typename VectorStateType>
    StateType k( const StateType T,
		 const VectorStateType& mass_fractions ) const;

    template <typename StateType, typename VectorStateType>
    void mu_and_k( const StateType T,
		   const VectorStateType& mass_fractions,
		   StateType& mu, StateType& k ) const;

    //! Helper function to reduce code duplication.
    /*! Populates species viscosities and the intermediate \chi variable
        needed for Wilke's mixing rule. */
    template <typename StateType>
    void compute_mu_chi( const StateType T,
			 VectorStateType& mu,
			 VectorStateType& chi ) const;

    //! Helper function to reduce code duplication.
    /*! Computes the intermediate \phi variable needed for Wilke's mixing rule.  */
    template <typename StateType>
    StateType compute_phi( const VectorStateType& mu,
			   const VectorStateType& chi,
			   const unsigned int s ) const;

  protected:

    const WilkeMixture<CoeffType>& _mixture;

    const MixtureViscosity<Viscosity,CoeffType>& _viscosity;

    const ThermalConductivity& _conductivity;

  private:

    WilkeEvaluator();

  };

  template<class V, class T, class CoeffType>
  WilkeEvaluator<V,T,CoeffType>::WilkeEvaluator( const WilkeMixture<CoeffType>& mixture,
						 const MixtureViscosity<V,CoeffType>& viscosity,
						 const MixtureThermalConductivity<T,CoeffType>& conductivity )
    : _mixture(mixture),
      _viscosity(viscosity),
      _conductivity(conductivity)
  {
    return;
  }

  template<class V, class T, class CoeffType>
  WilkeEvaluator<CoeffType,V,T>::~WilkeEvaluator()
  {
    return;
  }

  template<class V, class T, class CoeffType>
  template <typename StateType, typename VectorStateType>
  StateType WilkeEvaluator<CoeffType,V,T>::mu( const StateType T,
					       const VectorStateType& mass_fractions )
  {
    StateType mu_mix = zero_clone(T);
    
    VectorStateType mu  = zero_clone(mass_fractions);
    VectorStateType chi = zero_clone(mass_fractions);
    
    this->compute_mu_chi( T, mass_fractions, mu, chi );

    for( unsigned int s = 0; s < this->n_species(); s++ )
      {
	StateType phi_s = this->compute_phi( mu, chi, s );
	
	// Now compute phi_s, chi_s
	mu_mix += mu[s]*chi[s]/phi_s
      }

    return mu_mix;
  }

  template<class V, class T, class CoeffType>
  template <typename StateType>
  StateType WilkeEvaluator<V,T,CoeffType>::k( const StateType T,
					      const VectorStateType& mass_fractions )
  {
    antioch_not_implemented();

    StateType k = zero_clone(T);

    return k;
  }

  template<class V, class T, class CoeffType>
  template <typename StateType>
  void WilkeEvaluator<V,T,CoeffType>::mu_and_k( const StateType T,
						const VectorStateType& mass_fractions,
						StateType& mu, StateType& k )
  {
    antioch_not_implemented();

    mu = zero_clone(T);
    k  = zero_clone(T);

    return;
  }

  template <typename StateType>
  void WilkeEvaluator<V,T,CoeffType>::compute_mu_chi( const StateType T,
						      const VectorStateType& mass_fractions
						      VectorStateType& mu,
						      VectorStateType& chi ) const
  {
    const StateType M = _viscosity.chemical_mixture().M(mass_fractions);

    // Precompute needed quantities
    // chi_s = w_s*M/M_s
    for( unsigned int s = 0; s < this->n_species(); s++ )
      {
	mu[s] = _viscosity(s,T);
	chi[s] = mass_fractions[s]*M/_viscosity.chemical_mixture().M(s);
      }

    return;
  }

  template <typename StateType>
  StateType WilkeEvaluator<V,T,CoeffType>::compute_phi( const VectorStateType& mu,
							const VectorStateType& chi,
							const unsigned int s ) const
  {
    /* We initialize to the first iterate and loop starting from 1
       since some StateTypes have a hard time initializing from
       a constant. */
    // phi_s = sum_r (chi_r*(1+sqrt(mu_s/mu_r)*(Mr/Ms)^(1/4))^2)/(8*(1+Ms/Mr))
    const StateType dummy = 1.0 + std::sqrt(mu[s]/mu[0])*_mixture.Mr_Ms_to_the_one_fourth(0,s);
    StateType phi_s = chi[0]*numerator*numerator/_mixture.denom(0,s);

    for(unsigned int r = 1; r < this->n_species(); r++ )
      {
	const StateType numerator = 1.0 + std::sqrt(mu[s]/mu[r])*_mixture.Mr_Ms_to_the_one_fourth(r,s);
	phi_s += chi[r]*numerator*numerator/_mixture.denom(r,s);
      }

    return phi_s;
  }

} // end namespace Antioch

#endif // ANTIOCH_WILKE_EVALUATOR_H