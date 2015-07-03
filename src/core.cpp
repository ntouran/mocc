#include "core.hpp"

#include <string>
#include <sstream>
#include <iostream>

#include "error.hpp"
#include "string_utils.hpp"

namespace mocc {
    Core::Core() {
        nx_ = 0;
        ny_ = 0;
    }
    Core::Core( const pugi::xml_node &input, 
                const std::map<int, UP_Assembly_t> &assemblies):
        nx_( input.attribute("nx").as_int(-1) ),
        ny_( input.attribute("ny").as_int(-1) )
    {
        // Make sure that we read the a proper ID
        if ( nx_ < 1 | ny_ < 1 ) {
            Error("Invalid core dimensions.");
        }

        // Read in the assembly IDs
        std::string asy_str = input.child_value();
        std::stringstream inBuf( trim(asy_str) );
        std::vector<int> asy_vec;
        for ( int i=0; i<nx_*ny_; ++i ) {
            int id;
            inBuf >> id;
            asy_vec.push_back( id );
            if (inBuf.fail()) {
                Error("Trouble reading assembly IDs in core specification.");
            }
        }
        // Store references to the assemblies in a 2D array. Make sure to flip
        // the y-index to get it into lower-left origin
        assemblies_.resize( nx_*ny_ );
        for ( int iy=0; iy<ny_; iy++ ) {
            int row = ny_ - iy - 1;
            for ( int ix=0; ix<nx_; ix++ ) {
                int col = ix;
                Assembly* asy_p = 
                    assemblies.at( asy_vec[(ny_-iy-1)*nx_+ix] ).get();
                assemblies_[row*nx_ + col] = asy_p;
            }
        }

        // Check to make sure that the assemblies all fit together
        // Main things to check are that they all have the same number of planes
        // and that the heights of each plane match
        int nz = (*assemblies_.begin())->nz();
        for (auto asy = assemblies_.begin(); asy != assemblies_.end(); ++asy) {
            if ( (*asy)->nz() != nz ) {
                Error("Assemblies in the core have incompatible numbers of planes.");
            }
        }

        for( int i=0; i<nz; i++ ) {
            float_t hz = (*assemblies_.begin())->hz(i);
            for( auto asy = assemblies_.begin(); 
                 asy != assemblies_.end(); ++asy ) {
                if ((*asy)->hz(i) != hz) {
                    Error("Assemblies have incompatible plane heights in core.");
                }
            }
        }

        // Get the total number of pins along each dimension
        npinx_ = 0;
        for (int i=0; i<nx_; i++) {
            npinx_ += this->at(i, 0)->nx();
        }
        npiny_ = 0;
        for (int i=0; i<ny_; i++) {
            npiny_ += this->at(0, i)->ny();
        }
        std::cout << "core dimensions in pins " << npinx_ << " " << npiny_ << std::endl;
    }

    Core::~Core() {
    }
}
