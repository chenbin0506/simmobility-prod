/**
 * \file constants.h
 * Constant definitions. Most of these will become configurable parameters later.
 * \note Re-name to constants.hpp if we later decide to keep some constants; the .h extension
 * is used to indicate a temporary file.
 *
 * \par
 * ~Seth
 */


#pragma once

//namespace sim_mob {} //This is a temporary file, so it exists outside the namespace


//Sizes of workgroups. (Note that enums are allowed to overlap values)
enum WORKGROUP_SIZES {
	WG_TRIPCHAINS_SIZE = 4,       ///<Number of trip chain workers in group.
	WG_CREATE_AGENT_SIZE = 3,     ///<Number of agent creation workers in group.
	WG_CHOICESET_SIZE = 6,        ///<Number of choice set workers in group.
	WG_VEHICLES_SIZE = 5,         ///<Number of vehicle workers in group.
	WG_AGENTS_SIZE = 5,           ///<Number of agent workers in group.
	WG_SIGNALS_SIZE = 2,          ///<Number of signal workers in group.
	WG_SHORTEST_PATH_SIZE = 10,   ///<Number of shortest path workers in group.
};



/**
 * Declaration of a "trivial" function, which does nothing and returns a trivial conditional.
 * Used to indicate future functionality.
 */
bool trivial(unsigned int id);



/**
 * Temporary definition of nullptr. The new standard (C++11) will have "nullptr" as a builtin
 * keyword with special properties. So, I'm defining a class here with some of "nullptr"'s properties.
 * If we use, e.g.:
 *    #include "constants.h"
 *    in* x = nullptr;
 *
 * ...then when Tile-GCC supports nullptr, we can simply delete this temporary class and
 * recompile with _no_ other code changes.
 *
 * \note
 * Feel free to use "NULL" or "0" if you're not comfortable with nullptr.
 */
const class {                // nullptr is a const object.
public:
	//Convertible to any type of null non-member pointer.
	template<class T> operator T*() const { return 0; }

	//Covnertible to any type of null member pointer.
	template<class C, class T> operator T C::*() const { return 0; }

private:
	//Cannot take the address of nullptr.
	void operator&() const;

} nullptr = {};  //Single instance, named "nullptr"


