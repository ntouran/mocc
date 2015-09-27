#pragma once

#include "pugixml.hpp"

#include "angular_quadrature.hpp"
#include "correction_data.hpp"
#include "global_config.hpp"
#include "moc_sweeper_2d3d.hpp"
#include "sn_sweeper_cdd.hpp"
#include "source_2d3d.hpp"

namespace mocc {
    /**
     * This is an implementation of the 2D3D method. Each plane is treated with
     * a 2-D MoC sweeper, which produces the correction factors needed to treat
     * the entire system with a 3-D corrected diamond difference Sn sweeper.
     */
    class PlaneSweeper_2D3D: public TransportSweeper {
    public:
        PlaneSweeper_2D3D( const pugi::xml_node &input, const CoreMesh &mesh );

        void sweep( int group );

        void initialize();

        void get_pin_flux( int ig, VecF &flux ) const;

    
        void output( H5File& file ) const;

        void homogenize( CoarseData &data ) const {
            
        }

        /** 
         * Associate the sweeper with a source. This has to do a little extra
         * work, since the Sn sweeper needs its own source.
         */
        virtual void assign_source( const Source* source ) {
            assert( source != nullptr );
            source_ = source;
            moc_sweeper_.assign_source( source );
            /// \todo this static_cast is scary. Maybe think about relaxing the
            /// ownership of the source by FSS and allow the TS to figure out the
            /// types more explicitly...
            const Source_2D3D * s = static_cast<const Source_2D3D*>(source);
            sn_sweeper_.assign_source( s->get_sn_source() );

        }

        UP_Source_t create_source() const {
            return UP_Source_t( new Source_2D3D( moc_sweeper_, sn_sweeper_ ) ); 
        }

        SP_XSMeshHomogenized_t get_homogenized_xsmesh() {
            return sn_sweeper_.get_homogenized_xsmesh();
        }

        void calc_fission_source( float_t k, ArrayX &fission_source ) const;

        float_t total_fission( bool old ) const;

        /**
         * Defer to the MoC and Sn sweepers.
         */
        void store_old_flux() {
            moc_sweeper_.store_old_flux();
            sn_sweeper_.store_old_flux();
            return;
        }

        void set_coarse_data( CoarseData *cd ) {
            coarse_data_ = cd;
            moc_sweeper_.set_coarse_data( cd );
            sn_sweeper_.set_coarse_data( cd );
        }

    private:
        SnSweeper_CDD sn_sweeper_;
        MoCSweeper_2D3D moc_sweeper_;
        AngularQuadrature ang_quad_;
        CorrectionData corrections_;
    };
}