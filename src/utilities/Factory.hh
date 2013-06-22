//----------------------------------*-C++-*-----------------------------------//
/**
 *  @file  Factory.hh
 *  @brief Factory class definition
 *  @note  Copyright (C) 2013 Jeremy Roberts
 */
//----------------------------------------------------------------------------//

#ifndef detran_utilities_FACTORY_HH_
#define detran_utilities_FACTORY_HH_

#include "SP.hh"
#include <map>
#include <string>

namespace detran_utilities
{

/**
 *  @class Factory
 *  @brief Object factory template for creating an @ref SP to a derived type
 *
 *  This template is inspired by Alexandrescu's class of the same name
 *  <em> Modern C++ Design </em>.  However, it has been simplified
 *  to use one template parameter by assuming a string key and making the
 *  definition of an abstract object slightly more restrictive.
 *
 *  @tparam   B     Base class
 */
/**
 *  @example utilities/test/test_Factory.cc
 *
 *  Test of Factory
 */
template <class B>
class Factory
{

public:

  //--------------------------------------------------------------------------//
  // TYPEDEFS
  //--------------------------------------------------------------------------//

  typedef typename B::CreateFunction              CreateFunction;
  typedef std::map<std::string, CreateFunction>   CreateFunctionMap;
  typedef typename CreateFunctionMap::value_type  CreateFunctionMapPair;

  //--------------------------------------------------------------------------//
  // PUBLIC FUNCTIONS
  //--------------------------------------------------------------------------//

  /// Create and/or return the single instance
  static Factory& Instance();
  /// Register a derived class, returning 'true' if successful
  static bool Register(const std::string &key, CreateFunction create);
  /// Is the key registered?
  static bool IsRegistered(const std::string &key);
  /// Get the factory creation function
  CreateFunction GetCreateFunction(const std::string &key);

private:

  //--------------------------------------------------------------------------//
  // DATA
  //--------------------------------------------------------------------------//

  /// Map of strings to creation function pointers
  CreateFunctionMap d_callbacks;

  //--------------------------------------------------------------------------//
  // IMPLEMENTATIONS
  //--------------------------------------------------------------------------//

  //@{
  /// All construction is private
  Factory(){};
  Factory(Factory const&){};
  Factory& operator=(Factory const&){return Factory();};
  //@}

  /// Function that actually registers the key and function
  bool Register_private(const std::string &key, CreateFunction create);

};

/// Convenience macro for registering derived class classes
#define REGISTER_CLASS(B, D, S) \
        const bool D##_registered = Factory<B>::Register(S, Create<D>);

} // end namespace detran_utilities

#include "Factory.i.hh"

#endif /* detran_utilities_FACTORY_HH_ */

//----------------------------------------------------------------------------//
//              end of Factory.hh
//----------------------------------------------------------------------------//
