#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <complex>
#include "header.hpp"
#include "penning.hpp"
#include "particle.hpp"

std::complex<double> f(double t, double x0, double z0, double v0, double omega_plus, double omega_minus){
    double A_plus = (v0 + omega_minus*x0)/(omega_minus + omega_plus);
    double A_minus = (v0 + omega_plus*x0)/(omega_minus + omega_plus);
    std::complex<double> f_plus = std::polar(A_plus, -omega_plus*t);
    std::complex<double> f_minus = std::polar(A_minus, -omega_minus*t);
    return f_plus + f_minus;
}


int main()
{
    // particle parameters
    const double q = 1.,  m = 40.078;

    // trap parameters
    const double B0 = 96.5, V0 = 2.41 * 1e+6, d = 500;
    const bool mutual_interactions = false;

    // steps, boundary t values
    const int n = 32000;
    const double t_max = 50., t_min = 0., h = (t_max - t_min) / n;

    // initial values
    const int x0 = 20, z0 = 20, v0 = 25;

    // analytical solution parameters
    const double omega0 = q*B0/m, omega2_z = 2*q*V0/(m*d*d), discriminant=std::sqrt(omega0*omega0 - 2*omega2_z);
    const double omega_plus = (omega0+discriminant)*0.5, omega_minus = (omega0-discriminant)*0.5;
    std::complex<double> sol_anal;
    double x_anal, y_anal, z_anal, x_obs, y_obs, z_obs, err_x, err_y, err_z;

    // print parameters
    const int width = 12;
    const int prec = 12;

    // time vector
    arma::vec t(n + 1);

    // define empty system of particles
    std::vector<Particle> particles;

    // create empty trap
    PenningTrap trap(B0, V0, d, particles, mutual_interactions);

    // add one particle to the trap
    trap.add_particle(Particle(q, m, arma::vec(std::to_string(x0)+" 0 "+ std::to_string(z0)), arma::vec("0 "+std::to_string(v0)+" 0")) );

    // open file in order to save to file (t,x(t),y(t),z(t))
    std::string filename = "err_"+ std::to_string(n) + ".txt";

    std::ofstream ofile;
    ofile.open(filename);

    // time loop
    t(0) = t_min;
    for (int i = 0; i < n + 1; i++){
        t(i) = t_min + i * h;

        sol_anal = f(t(i), x0, z0, v0, omega_plus, omega_minus);
        x_anal = std::real(sol_anal);
        y_anal = std::imag(sol_anal);
        z_anal = z0*cos(std::sqrt(omega2_z)*t(i));
        
        x_obs = trap.particles[0].r(0);
        y_obs = trap.particles[0].r(1);
        z_obs = trap.particles[0].r(2);
        //vx_obs = trap.particles[0].v(0);
        //vy_obs = trap.particles[0].v(1);
        //vz_obs = trap.particles[0].v(2);

        err_x = std::fabs(x_anal - x_obs)/x_anal;
        err_y = std::fabs(y_anal - y_obs)/y_anal;
        err_z = std::fabs(z_anal - z_obs)/z_anal;

        ofile   << " " << scientific_format(t(i), width, prec)
                << " " << scientific_format(err_x, width, prec)
                << " " << scientific_format(err_y, width, prec)
                << " " << scientific_format(err_z, width, prec)
                //<< " " << scientific_format(trap.particles[0].v(0), width, prec)
                //<< " " << scientific_format(trap.particles[0].v(1), width, prec)
                //<< " " << scientific_format(trap.particles[0].v(2), width, prec) 
                << std::endl;
        trap.evolve_RK4(h);
    }

    // close file
    ofile.close();

    // all good
    return 0;
}