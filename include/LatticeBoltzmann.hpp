#ifndef LATTICE_BOLTZMANN_HPP_
#define LATTICE_BOLTZMANN_HPP_
#include <vector>
#include "CollisionModel.hpp"
#include "LatticeModel.hpp"

class LatticeBoltzmann {
 public:
  /**
   * Constructor: Creates lattice
   * \param t_total total time of simulation
   * \param obstacles_pos lattice containing position of obstacles
   * \param is_ns NS equation toggle
   * \param is_cd CD equation toggle
   * \param is_taylor workaround to toggle bounceback boundary condition for
   *        left and right edge for taylor analytical test
   * \param is_instant instantaneous source togle
   * \param has_obstacles obstacles toggle
   * \param lm lattice model to take care of dimensions, directions, number of
   *        rows, number of columns, space step, time step, omega, discrete
   *        velocity vectors
   * \param ns Collision model for NS equation
   * \param cd Collision model for CD equation
   */
  LatticeBoltzmann(double t_total
    , double u_lid
    , const std::vector<std::vector<std::size_t>> &obstacles_pos
    , bool is_ns
    , bool is_cd
    , bool is_taylor
    , bool is_lid
    , bool is_instant
    , bool has_obstacles
    , LatticeModel &lm
    , CollisionModel &cm);

  ~LatticeBoltzmann() = default;

  LatticeBoltzmann(const LatticeBoltzmann&) = default;

  /**
   * Computes boundary condition for the lattice, bounceback for top and bottom
   * edge, periodic for left and right edge.
   * \param lattice 2D vector containing distribution functions
   * \return 2D vector containing the boundary nodes in the order: left, right,
   *         top, bottom, corners
   */
  std::vector<std::vector<double>> BoundaryCondition(
      const std::vector<std::vector<double>> &lattice);

  /**
   * Assembles lattice and boundary nodes into a bigger lattice with border,
   * then performs streaming step.
   * \param lattice 2D vector containing distribution functions
   * \param boundary 2D vector containing boundary nodes
   * \return post-stream lattice (no border)
   */
  std::vector<std::vector<double>> Stream(
      const std::vector<std::vector<double>> &lattice
    , const std::vector<std::vector<double>> &boundary);

  /**
   * Does Zou-He velocity BC for lid-driven flow
   * \param lattice 2D vector containing distribution functions
   */
  void BoundaryLid(std::vector<std::vector<double>> &lattice);

  std::vector<std::vector<double>> StreamImmersed(
      const std::vector<std::vector<double>> &lattice);

  /**
   * Performs one cycle of evolution equation, computes the relevant macroscopic
   * properties such as velocity and density
   */
  void TakeStep();

  /**
   * Performs numerous cycles of the evolution equation based on total time of
   * the simulation and time step of the simulation, does not output any files
   * to Cmgui
   */
  void RunSim();

  /**
   * Performs numerous cycles of the evolution equation based on total time of
   * the simulation and time step of the simulation, specifies a file to output
   * using Cmgui
   * \param lattice the be printed using Cmgui (distribution functions or
   *       velocity)
   */
  void RunSim(std::vector<std::vector<double>> &lattice);

  /**
   * NS distribution function stored row-wise in a 2D vector.
   */
  std::vector<std::vector<double>> df;

  /**
   * Boundary nodes for NS lattice
   */
  std::vector<std::vector<double>> boundary_f;

  /**
   * Lattice containing obstacles
   */
  std::vector<bool> obstacles;

 private:
  // 6  2  5  ^
  //  \ | /   |
  // 3--0--1  |
  //  / | \   |
  // 7  4  8  +------->
  enum Directions {
    E = 1,
    N,
    W,
    S,
    NE,
    NW,
    SW,
    SE
  };
  /**
   * Checks input parameters to ensure there's not invalid values
   * \return true if there is invalid: 0 in any values, both is_ns_ and is_cd_
   *         are false
   *         false if all input values are valid
   */
  bool CheckParameters();
  // input parameters
  double total_time_;
  double u_lid_;
  bool is_ns_;
  bool is_cd_;
  bool is_taylor_;
  bool is_lid_;
  bool is_instant_;
  bool has_obstacles_;
  // LatticeModel to take care of dims, dirs, rows, cols and discrete e vectors
  // by reference, similar to by pointer
  // https://stackoverflow.com/questions/9285627/is-it-possible-to-pass-derived-
  // classes-by-reference-to-a-function-taking-base-cl
  LatticeModel &lm_;
  CollisionModel &cm_;
  // Collision models to take care of eq, rho, source
//  CollisionNS &ns_;
//  CollisionCD &cd_;
};
#endif  // LATTICE_BOLTZMANN_HPP_
