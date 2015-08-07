#include "transport_sweeper_factory.hpp"

#include <string>

#include "error.hpp"
#include "mesh.hpp"
#include "moc_sweeper.hpp"
#include "sn_sweeper.hpp"

namespace mocc {
    UP_Sweeper_t TransportSweeperFactory( const pugi::xml_node &input,
            const CoreMesh &mesh ) {
        // Check the input XML for which type of sweeper to make
        std::string type = input.child("sweeper").attribute("type").value();
        if( type == "moc" ) {
            SP_XSMesh_t xsm( new XSMesh(mesh) );
            UP_Sweeper_t ts( new MoCSweeper( input.child("sweeper"), mesh, 
                        xsm ) );
            return ts;
        } else if ( type == "sn" ) {
            SP_XSMesh_t xsm( new XSMesh(mesh) );
            UP_Sweeper_t ts( new SnSweeper( input.child("sweeper"), mesh, 
                        xsm ) );
            return ts;
        } else {
            throw EXCEPT("Failed to detect a valid sweeper type.");
        }
    }
}
