#include "xs_mesh.hpp"

#include <iostream>

#include "global_config.hpp"

using std::cout;
using std::endl;

namespace mocc {
    
    XSMesh::XSMesh( const CoreMesh& mesh ) {
        const MaterialLib& mat_lib = mesh.mat_lib();

        // Assume the same number of groups as the source material library
        ng_ = mat_lib.n_grp();

        // Get energy group bounds
        eubounds_ = mat_lib.g_bounds();

        // Make a list of the material ids, to get back into a dense index space
        VecI matv;
        for( auto &mi: mat_lib.materials()) {
            // mi is a std::pair<int, Material>&
            matv.push_back(mi.first);
        }

        // Make a temporary map from the material ID as specified in the input
        // file/material library to its index in the XS mesh
        std::map<int, int> mat_map;
        int ixsreg = 0;
        for ( auto &i: matv ){
            mat_map[i] = ixsreg++;
        }

        // loop over all of the pins in the CoreMesh and set up the XSMesh
        std::vector<VecI> fsrs(mat_lib.n_materials());
        int ireg = 0;
        for( auto pini=mesh.begin_pin(); pini!=mesh.end_pin(); ++pini ) {
            const PinMesh& pm = (*pini)->mesh();
            const VecI& mat_ids = (*pini)->mat_ids();
            ixsreg = 0;
            for( auto &mat_id: mat_ids ) {
                int mat_index = mat_map[mat_id];
                // post-increment pushes value, then increments
                for( int reg=0; reg<pm.n_fsrs(ixsreg); reg++) {
                    fsrs[mat_index].push_back(ireg++);
                }
                ixsreg++;
            }
        }

        // Calculate the necessary cross sections and store them in the
        // XSMesh-local arrays
        for( unsigned int imat=0; imat<mat_lib.n_materials(); imat++ ) {
            unsigned int lib_key = matv[imat];
            const Material& mati = *mat_lib.materials().at(lib_key);
            // This is a bit of a hack until i use Eigen vectors or something
            // homebrew for storing these cross sections. need to add total
            // scattering to absorption to get xstr
            VecF tmp_tr(ng_, 0.0);
            for( unsigned int ig=0; ig<ng_; ig++ ) {
                tmp_tr[ig] = mati.xsab()[ig]+mati.xssc().out(ig);
            }
            regions_.emplace_back( fsrs[imat], 
                    tmp_tr,
                    mati.xsnf(), 
                    mati.xsch(), 
                    mati.xsf(), 
                    mati.xssc() );
        }

        return;
    }
}
