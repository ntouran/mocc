#include "core_mesh.hpp"

#include <iostream>
#include <string>
#include <sstream>

#include "files.hpp"
#include "error.hpp"
#include "string_utils.hpp"


using std::cout;
using std::endl;
using std::stringstream;


namespace mocc {
    CoreMesh::CoreMesh(pugi::xml_node &input) {
        // Parse meshes
        for (pugi::xml_node mesh = input.child( "mesh" );
             mesh; 
             mesh = mesh.next_sibling( "mesh" )) {
            LogFile << "Parsing new pin mesh: ID=" 
            << mesh.attribute( "id" ).value() << endl;
            UP_PinMesh_t pm( PinMeshFactory( mesh ) );
            pin_meshes_.emplace(pm->id(), std::move( pm ) );
        }
        
        // Parse Material Library
        std::string matLibName = 
            input.child( "material_lib" ).attribute( "path" ).value();
        cout << "Found material library specification: " << matLibName << endl;
        FileScrubber matLibFile( matLibName.c_str(), "!" );
        mat_lib_ = MaterialLib( matLibFile );
        
        // Parse material IDs
        for (pugi::xml_node mat = input.child( "material_lib" ).child( "material" ); 
                mat; mat = mat.next_sibling( "material" )){
            cout << mat.attribute( "id" ).value() << " "
                 << mat.attribute( "name" ).value() << endl;
            mat_lib_.assignID( mat.attribute( "id" ).as_int(),
                               mat.attribute( "name" ).value() );
        }
        
        // Parse pins
        for ( pugi::xml_node pin = input.child( "pin" ); pin; 
                pin = pin.next_sibling( "pin" ) ) {
            // Get pin ID
            int pin_id = pin.attribute( "id" ).as_int( -1 );
            if ( pin_id == -1 ) {
                Error( "Failed to read pin ID." );
            }

            // Get pin mesh ID
            int mesh_id = pin.attribute( "mesh" ).as_int( -1 );
            if ( mesh_id == -1) {
                Error( "Failed to read pin mesh ID." );
            }
            if ( pin_meshes_.count( mesh_id ) == 0 ) {
                Error( "Invalid pin mesh ID." );
            }

            // Get material IDs
            std::string mats_in = pin.child_value();
            stringstream inBuf( trim( mats_in ) );
            int mat_id;
            VecI mats;

            while( !inBuf.eof() ){
                mat_id = -1;
                inBuf >> mat_id;

                mats.push_back( mat_id );
            }
            if( inBuf.fail() ){
                Error( "Trouble reading material IDs in pin definition." );
            }
            if ( mats.size() != pin_meshes_[pin_id]->n_xsreg() ) {
                Error( "Wrong number of materials specified in pin definition" );
            }

            // Construct the pin and add to the map
            UP_Pin_t pin_p( new Pin( pin_id, pin_meshes_[mesh_id].get(), mats   ) );
            pins_.emplace( pin_id, std::move(pin_p) );
        }

        // Parse lattices
        for ( pugi::xml_node lat = input.child( "lattice" ); lat;
                lat = input.next_sibling( "lattice" )) {
            Lattice lattice( lat, pins_ );
            lattices_.emplace( lattice.id(), lattice );
        }

        // Parse assemblies
        for ( pugi::xml_node asy = input.child("assembly"); asy;
                asy = input.next_sibling("assembly") ) {
            int asy_id = asy.attribute("id").as_int();
            UP_Assembly_t asy_p( new Assembly( asy, lattices_ ) );
            assemblies_.emplace( asy_p->id(), std::move(asy_p) );
        }

        // Parse core
        core_ = Core( input.child("core"), assemblies_ );

        nx_ = core_.nx();
        ny_ = core_.ny();
        nz_ = core_.nz();

        // Calculate the total core dimensions
        hx_ = 0.0;
        for ( int ix=0; ix<core_.nx(); ix++ ) {
            hx_ += core_.at(ix, 0)->hx();
        }
        hy_ = 0.0;
        for ( int iy=0; iy<core_.ny(); iy++ ) {
            hy_ += core_.at(iy, 0)->hy();
        }

        // Determine the set of geometricaly-unique axial planes
        std::vector< std::vector<int> > unique;
        for ( int iz=0; iz<nz_; iz++)

        return;
    }

    CoreMesh::~CoreMesh() {
        return;
    }
}
