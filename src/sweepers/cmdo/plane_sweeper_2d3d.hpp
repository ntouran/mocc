#pragma once

#include "pugixml.hpp"
#include <blitz/array.h>

#include "core/angular_quadrature.hpp"
#include "core/global_config.hpp"
#include "core/output_interface.hpp"

#include "sn/sn_sweeper_variant.hpp"

#include "correction_data.hpp"
#include "moc_sweeper_2d3d.hpp"
#include "sn_sweeper_cdd.hpp"
#include "sn_sweeper_factory_cdd.hpp"
#include "source_2d3d.hpp"

namespace mocc { namespace cmdo {
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

        /**
         * \brief \copybrief TransportSweeper::get_pin_flux_1g()
         */
        void get_pin_flux_1g( int ig, ArrayB1 &flux ) const;

        /**
         * \brief \copybrief TransportSweeper::set_pin_flux_1g()
         *
         * Delegate to the subbordinate \ref sn::SnSweeper and \ref
         * moc::MoCSweeper.  Return the error from the MoC sweeper.
         */
        real_t set_pin_flux_1g( int group, const ArrayB1 &pin_flux ) {
            sn_sweeper_->set_pin_flux_1g( group, pin_flux );
            real_t diff = moc_sweeper_.set_pin_flux_1g( group, pin_flux );

            return diff;
        }

        /**
         * \brief \copybrief HasOutput::output()
         */
        void output( H5Node &file ) const;

        /**
         * \brief \copybrief TransportSweeper::assign_source()
         *
         * Associate the sweeper with a source. This has to do a little extra
         * work, since the Sn sweeper needs its own source.
         */
        virtual void assign_source( Source* source ) {
            assert( source != nullptr );
            source_ = source;
            moc_sweeper_.assign_source( source );
            /// \todo this static_cast is scary. Maybe think about relaxing the
            /// ownership of the source by FSS and allow the TS to figure out
            /// the types more explicitly...
            Source_2D3D *s = static_cast<Source_2D3D*>(source);
            sn_sweeper_->assign_source( s->get_sn_source() );
        }

        /**
         * \breif \copybrief TransportSweeper::create_source()
         *
         * Create a Source_2D3D object instead of the standard Source class.
         */
        UP_Source_t create_source( const pugi::xml_node &input ) const {
            std::cout << "creating 2d3d source" << std::endl;
            return UP_Source_t( new Source_2D3D( moc_sweeper_, *sn_sweeper_ ) );
        }

        SP_XSMeshHomogenized_t get_homogenized_xsmesh() {
            return sn_sweeper_->get_homogenized_xsmesh();
        }

        /**
         * \brief \copybrief TransportSweeper::calc_fission_source()
         *
         * Override the default \ref TransportSweeper implementation to call the
         * method on one of the sub-sweepers.
         */
        void calc_fission_source( real_t k, ArrayB1 &fission_source ) const {
            moc_sweeper_.calc_fission_source( k, fission_source );
            return;
        }

        /**
         * \brief \copybrief TransportSweeper::total_fission()
         *
         * Override the default \ref TransportSweeper implementation to call the
         * method on one of the sub-sweepers. For now, using the MoC
         * implementation, since it's the finer mesh, generally speaking
         */
        real_t total_fission( bool old ) const {
            return moc_sweeper_.total_fission( old );
        }

        /**
         * \brief \copybrief TransportSweeper::store_old_flux()
         *
         * Defer to the MoC and Sn sweepers.
         */
        void store_old_flux() {
            moc_sweeper_.store_old_flux();
            sn_sweeper_->store_old_flux();
            return;
        }

        /**
         * \brief \copybrief TransportSweeper::set_coarse_data()
         *
         * Delegate to subordinate sweepers.
         */
        void set_coarse_data( CoarseData *cd ) {
            coarse_data_ = cd;
            moc_sweeper_.set_coarse_data( cd );
            sn_sweeper_->set_coarse_data( cd );
        }

    private:
        // Parse the various options from the XML
        void parse_options( const pugi::xml_node &input );
        // Calculate transverse leakage based on the state of the coarse_data_
        // and apply to the MoC sweeper's source.
        void add_tl( int group );


        const CoreMesh& mesh_;
        CDDPair_t pair_;
        UP_SnSweeper_t sn_sweeper_;
        std::shared_ptr<CorrectionData> corrections_;
        MoCSweeper_2D3D moc_sweeper_;
        AngularQuadrature ang_quad_;
        ArrayB2 tl_;

        // Sn-MoC residuals by group sweep
        std::vector<VecF> sn_resid_;

        // Options! Buttons and knobs!!!
        bool expose_sn_; // When asking the sweeper for pin flux, which one?
        bool do_snproject_;
        bool do_tl_;
        int n_inactive_moc_;
        int i_outer_;
        int moc_modulo_;
    };
} } // Namespace mocc::cmdo
