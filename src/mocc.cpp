#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <iomanip>
#include <omp.h>
#include <sstream>
#include <string>

#include "mocc-core/core_mesh.hpp"
#include "mocc-core/error.hpp"
#include "mocc-core/files.hpp"
#include "mocc-core/global_config.hpp"
#include "mocc-core/h5file.hpp"
#include "mocc-core/transport_sweeper.hpp"

#include "input_proc.hpp"


using std::cout;
using std::cin;
using std::endl;

using namespace mocc;

// The global top-level solver
SP_Solver_t solver;

// Global core mesh
SP_CoreMesh_t mesh;

// Generate output from the solver
void generate_output() {
    HDF::H5File outfile( "out.h5", "w" );
    solver->output( outfile.get() );
}

// Signal handler for SIGINT. Calls output() and quits
void int_handler(int p) {
    std::cout << "Caught SIGINT. Bailing." << std::endl;
    generate_output();
    std::exit(EXIT_FAILURE);
}


int main(int argc, char* argv[]){
    // Make sure we have an input file
    if(argc < 2){
        Error("No input file specified!");
    }

    std::signal( SIGINT, int_handler );

    try {
        auto time_begin = omp_get_wtime();
        std::string space = "                         ";
        std::cout << space << "01001101010011110100001101000011" << std::endl;
        std::cout << space << " __  __   _____   _____   _____" <<std::endl;
        std::cout << space << "|  \\/  | |  _  | /  __ \\ /  __ \\" << std::endl;
        std::cout << space << "| .  . | | | | | | /  \\/ | /  \\/" << std::endl;
        std::cout << space << "| |\\/| | | | | | | |     | |    " << std::endl;
        std::cout << space << "| |  | | \\ \\_/ / | \\__/\\ | \\__/ " << std::endl;
        std::cout << space << "\\_|  |_/  \\___/   \\____/  \\____/" << std::endl;
        std::cout << space << std::endl;
        std::cout << space << "01101101011011110110001101100011 " << std::endl;

        // Spin up the log file. For now, just use the name of the input file.
        StartLogFile(argv[1]);

#pragma omp parallel
        {
#pragma omp master
            {
                LogFile << "Running with " << omp_get_num_threads() << " treads"
                    << std::endl;
            }
        }


        // Parse the input file
        InputProc inProc(argv[1]);

        // Get an SP to the core mesh
        mesh = inProc.core_mesh();
        LogFile << *mesh << endl;

        // Pull a shared pointer to the top-level solver and make it go
        solver = inProc.solver();
        solver->solve();

        // Output stuff
        

        auto time_end = omp_get_wtime();
        std::cout << "Time: " << time_end - time_begin << " sec" << endl;
        LogFile   << "Time: " << time_end - time_begin << " sec" << endl;

        StopLogFile();
    }

    catch(Exception e) {
        cout << "Error:" << endl;
        cout << e.what();
        return 1;
    }
}
