#include "CollisionModel.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
#include "Algorithm.hpp"
#include "LatticeModel.hpp"

CollisionModel::CollisionModel(LatticeModel &lm
  , double initial_density)
  : edf {},
    rho {},
    skip {},
    lm_ (lm),
    tau_ {0},
    c_ {lm.GetLatticeSpeed()}
{
  const auto nx = lm_.GetNumberOfColumns();
  const auto ny = lm_.GetNumberOfRows();
  const auto nc = lm_.GetNumberOfDirections();
  const auto lat_size = nx * ny;
  edf.assign(lat_size, std::vector<double>(nc, 0.0));
  rho.assign(lat_size, initial_density);
  skip.assign(lat_size, false);
  ComputeEq();
}

CollisionModel::CollisionModel(LatticeModel &lm
  , const std::vector<double> &initial_density)
  : edf {},
    rho {initial_density},
    skip {},
    lm_ (lm),
    tau_ {0},
    c_ {lm.GetLatticeSpeed()}
{
  const auto nx = lm_.GetNumberOfColumns();
  const auto ny = lm_.GetNumberOfRows();
  const auto nc = lm_.GetNumberOfDirections();
  const auto lat_size = nx * ny;
  edf.assign(lat_size, std::vector<double>(nc, 0.0));
  skip.assign(lat_size, false);
  ComputeEq();
}

void CollisionModel::ComputeEq()
{
  auto nc = lm_.GetNumberOfDirections();
  auto nx = lm_.GetNumberOfColumns();
  auto ny = lm_.GetNumberOfRows();
  for (auto n = 0u; n < nx * ny; ++n) {
    double u_sqr = InnerProduct(lm_.u[n], lm_.u[n]);
    u_sqr /= 2.0 * cs_sqr_;
    for (auto i = 0u; i < nc; ++i) {
      double c_dot_u = InnerProduct(lm_.e[i], lm_.u[n]);
      c_dot_u /= cs_sqr_;
      edf[n][i] = lm_.omega[i] * rho[n] * (1.0 + c_dot_u * (1.0 + c_dot_u /
          2.0) - u_sqr);
    }  // i
  }  // n
}

std::vector<double> CollisionModel::ComputeRho(
    const std::vector<std::vector<double>> &df)
{
  std::vector<double> result(df.size(), 0.0);
  auto it_result = begin(result);
  for (auto node : df) (*it_result++) = GetZerothMoment(node);
  return result;
}

void CollisionModel::AddNodeToSkip(std::size_t n)
{
  skip[n] = true;
}
