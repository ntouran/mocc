/*
   Copyright 2016 Mitchell Young

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include "core_mesh.hpp"
#include "pugifwd.hpp"
#include "solver.hpp"

#include "mc/fission_bank.hpp"
#include "mc/particle_pusher.hpp"

namespace mocc {
    class MonteCarloEigenvalueSolver: public Solver {
    public:
        MonteCarloEigenvalueSolver( const pugi::xml_node &input,
                const CoreMesh &mesh );

        void solve();
        void step();

        void output( H5Node &node ) const override;

    private:
        // Data
        const CoreMesh &mesh_;
        const XSMesh xs_mesh_;
        ParticlePusher pusher_;
        int n_cycles_;
        int n_inactive_cycles_;
        int particles_per_cycle_;

        FissionBank fission_bank_;

        real_t k_eff_;
        real_t k_variance_;
        bool active_cycle_;
    };
} // namespace mocc