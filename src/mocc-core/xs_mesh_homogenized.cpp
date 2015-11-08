#include "xs_mesh_homogenized.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

using std::cout;
using std::cin;
using std::endl;

namespace mocc {
    XSMeshHomogenized::XSMeshHomogenized( const CoreMesh& mesh ):
        mesh_( mesh )
    {
        // Set up the non-xs part of the xs mesh
        eubounds_ = mesh_.mat_lib().g_bounds();
        ng_ = eubounds_.size();

        regions_ = std::vector<XSMeshRegion>( mesh_.n_pin() );
        
        int ipin = 0;
        int first_reg = 0;
        for( const auto &pin: mesh_ ) {
            // Use the lexicographically-ordered pin index as the xs mesh index.
            // This is to put the indexing in a way that works best for the Sn
            // sweeper as it is implemented now. This is really brittle, and
            // should be replaced with some sort of Sn Mesh object, which both
            // the XS Mesh and the Sn sweeper will use to handle indexing.
            auto pos = mesh.pin_position(ipin);
            int ireg = mesh.index_lex( pos );
            int ixsreg = mesh_.index_lex( pos );
            regions_[ixsreg] = this->homogenize_region( ireg, *pin );
            ipin++;
            first_reg += pin->n_reg();
        }
    }

    /**
     * Update the XS mesh, incorporating a new estimate of the scalar flux.
     */
    void XSMeshHomogenized::update( const ArrayF &flux ) {
        int ipin = 0;
        int first_reg = 0;
        for( const auto &pin: mesh_ ) {
            int ireg = mesh_.index_lex( mesh_.pin_position(ipin) );
            int ixsreg = ireg;
            regions_[ixsreg] = this->homogenize_region_flux( ireg, first_reg, 
                    *pin, flux);

            ipin++;
            first_reg += pin->n_reg();
        }
        return;
    }
    
    XSMeshRegion XSMeshHomogenized::homogenize_region( int i, 
            const Pin& pin) const {
        VecI fsrs( 1, i );
        VecF xstr( ng_, 0.0 );
        VecF xsnf( ng_, 0.0 );
        VecF xskf( ng_, 0.0 );
        VecF xsch( ng_, 0.0 );

        std::vector<VecF> scat( ng_, VecF(ng_, 0.0) );

        auto mat_lib = mesh_.mat_lib();
        auto &pin_mesh = pin.mesh();
        auto vols = pin_mesh.vols();

        for( size_t ig=0; ig<ng_; ig++ ) {
            int ireg = 0;
            int ixsreg = 0;
            real_t fvol = 0.0;
            for( auto &mat_id: pin.mat_ids() ) {
                auto mat = mat_lib.get_material_by_id(mat_id);
                const ScatteringRow& scat_row = mat.xssc().to(ig);
                int gmin = scat_row.min_g;
                int gmax = scat_row.max_g;
                real_t fsrc = 0.0;
                for( size_t igg=0; igg<ng_; igg++ ) {
                    fsrc += mat.xsnf()[igg];
                }
                for( size_t i=0; i<pin_mesh.n_fsrs(ixsreg); i++ ) {
                    fvol += vols[ireg] * fsrc;
                    xstr[ig] += vols[ireg] * mat.xstr()[ig];
                    xsnf[ig] += vols[ireg] * mat.xsnf()[ig];
                    xskf[ig] += vols[ireg] * mat.xskf()[ig];
                    xsch[ig] += vols[ireg] * fsrc * mat.xsch()[ig];

                    for( int igg=gmin; igg<=gmax; igg++ ) {
                        scat[ig][igg] += scat_row.from[igg-gmin] * vols[ireg];
                    }
                    ireg++;
                }
                ixsreg++;
            }

            xstr[ig] /= pin.vol();
            xsnf[ig] /= pin.vol();
            xskf[ig] /= pin.vol();
            if( fvol > 0.0 ) {
                xsch[ig] /= fvol;
            }

            for( auto &s: scat[ig] ) {
                s /= pin.vol();
            }

        }

        ScatteringMatrix scat_mat(scat);

        return XSMeshRegion( fsrs, xstr, xsnf, xsch, xskf, scat_mat );
    }

    XSMeshRegion XSMeshHomogenized::homogenize_region_flux( int i, 
            int first_reg, const Pin& pin, const ArrayF &flux ) const {

        size_t n_reg = mesh_.n_reg();

        // Set the FSRs to be one element, representing the coarse mesh index to
        // which this \ref XSMeshRegion belongs.
        VecI fsrs( 1, i );
        VecF xstr( ng_, 0.0 );
        VecF xsnf( ng_, 0.0 );
        VecF xskf( ng_, 0.0 );
        VecF xsch( ng_, 0.0 );

        std::vector<VecF> scat( ng_, VecF(ng_, 0.0) );

        auto mat_lib = mesh_.mat_lib();
        auto &pin_mesh = pin.mesh();
        auto vols = pin_mesh.vols();

        // Precompute the fission source in each region, since it is the
        // wieghting factor for chi
        VecF fs( pin_mesh.n_reg(), 0.0 );
        {
            int ixsreg = 0;
            for( auto &mat_id: pin.mat_ids() ) {
                auto mat = mat_lib.get_material_by_id(mat_id);
                for( size_t ig=0; ig<ng_; ig++ ) {
                    int ireg = first_reg; // pin-local region index
                    int ireg_local = 0;
                    for( size_t i=0; i<pin_mesh.n_fsrs(ixsreg); i++ ) {
                        fs[ireg_local] += mat.xsnf()[ig] * flux[ireg + ig*n_reg] *
                            vols[ireg_local];
                        ireg++;
                        ireg_local++;
                    }
                }
                ixsreg++;
            }
        }

        real_t fs_sum = 0.0;
        for( auto &v: fs ) {
            fs_sum += v;
        }

        for( size_t ig=0; ig<ng_; ig++ ) {
            real_t fluxvolsum = 0.0;
            VecF scatsum(ng_, 0.0);
            int ireg = first_reg; // global region index
            int ireg_local = 0; // pin-local refion index
            int ixsreg = 0;
            for( auto &mat_id: pin.mat_ids() ) {
                auto mat = mat_lib.get_material_by_id(mat_id);
                const ScatteringRow& scat_row = mat.xssc().to(ig);
                size_t gmin = scat_row.min_g;
                size_t gmax = scat_row.max_g;
                for( size_t i=0; i<pin_mesh.n_fsrs(ixsreg); i++ ) {
                    real_t v = vols[ireg_local];
                    real_t flux_i = flux[ireg + ig*n_reg];
                    fluxvolsum += v * flux_i;
                    xstr[ig] += v * flux_i * mat.xstr()[ig];
                    xsnf[ig] += v * flux_i * mat.xsnf()[ig];
                    xskf[ig] += v * flux_i * mat.xskf()[ig];
                    xsch[ig] += fs[ireg_local] * mat.xsch()[ig];

                    for( size_t igg=0; igg<ng_; igg++ ){
                        real_t fluxgg = flux[ireg + igg*n_reg];
                        scatsum[igg] += fluxgg * v;
                        if( (igg >= gmin) && (igg <= gmax) ) {
                            real_t scgg = scat_row.from[igg-gmin];
                            scat[ig][igg] += scgg * v * fluxgg;
                        }
                    }
                    ireg++;
                    ireg_local++;
                }
                ixsreg++;
            }

            for( size_t igg=0; igg<ng_; igg++ ) {
                if( scat[ig][igg] > 0.0 ) {
                    scat[ig][igg] /= scatsum[igg];
                }
            }

            xstr[ig] /= fluxvolsum;
            xsnf[ig] /= fluxvolsum;
            xskf[ig] /= fluxvolsum;
            if(fs_sum > 0.0) {
                xsch[ig] /= fs_sum;
            }
        }

        ScatteringMatrix scat_mat(scat);

        return XSMeshRegion( fsrs, xstr, xsnf, xsch, xskf, scat_mat );
    }
           

    void XSMeshHomogenized::output( H5::CommonFG *file ) const {
        file->createGroup( "/xsmesh" );
        file->createGroup( "/xsmesh/xstr" );
        file->createGroup( "/xsmesh/xsnf" );

        auto d = mesh_.dimensions();
        std::reverse(d.begin(), d.end());

        for( size_t ig=0; ig<ng_; ig++ ) {
            // Transport cross section
            VecF xstr( this->size(), 0.0 );
            VecF xsnf( this->size(), 0.0 );
            int i = 0;
            for( auto xsr: regions_ ) {
                xstr[i] = xsr.xsmactr()[ig];
                xsnf[i] = xsr.xsmacnf()[ig];
                i++;
            }
            {
                std::stringstream setname;
                setname << "/xsmesh/xstr/" << ig;
                HDF::Write( file, setname.str(), xstr, d );
            }
            {
                std::stringstream setname;
                setname << "/xsmesh/xsnf/" << ig;
                HDF::Write( file, setname.str(), xsnf, d );
            }
        }

        // scattering matrix
        VecF scat( regions_.size()*ng_*ng_, 0.0 );
        auto it = scat.begin();
        for( const auto &reg: regions_ ){
            auto reg_scat = reg.xsmacsc().as_vector();
            it = std::copy( reg_scat.begin(), reg_scat.end(), it );
        }
        VecI dims( 3 );
        dims[0] = regions_.size();
        dims[1] = ng_;
        dims[2] = ng_;

        HDF::Write( file, "/xsmesh/xssc", scat, dims );


        return;
    }

    
}
