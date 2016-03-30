#include "UnitTest++/UnitTest++.h"

#include <stdlib.h>
#include <cmath>
#include <iostream>

#include "core/exponential.hpp"

using namespace mocc;
using std::cout;
using std::endl;

TEST( exp )
{

    Exponential_Linear<10000> exp;
    cout << "Max error: " << exp.max_error() << endl;

    real_t max_err = 0.0;
    for( real_t x=-10.0; x<0.0; x+=0.1 ) {
        real_t exp_t = exp.exp(x);
        real_t exp_r = std::exp(x);
        max_err = std::max(max_err, std::abs(exp_r - exp_t));
        cout << x << " " << exp_r << " " << exp_t << endl;
        CHECK( std::abs(exp_r - exp_t) < 2e-8 );
    }

    cout << "max_err: " << max_err << endl;
}


TEST( exp_positive ) {
    Exponential_Linear<50000> exp(0.0, 10.0);

    cout << "max error from exp: " << exp.max_error() << std::endl;

    real_t max_err = 0.0;
    for( real_t x=0.0; x<10.0; x+=0.1 ) {
        real_t exp_t = exp.exp(x);
        real_t exp_r = std::exp(x);
        real_t err = std::abs(exp_r - exp_t)/exp_r;
        max_err = std::max(max_err, err);
        cout << x << " " << exp_r << " " << exp_t << " " << err << endl;
        CHECK( err < 2e-8 );
    }

    cout << "max_err: " << max_err << endl;
}

int main(int, const char*[]) {
    return UnitTest::RunAllTests();
}
