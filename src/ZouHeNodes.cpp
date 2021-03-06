#include "ZouHeNodes.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
#include "ValueNode.hpp"

ZouHeNodes::ZouHeNodes(LatticeModel &lm
  , CollisionModel &cm)
  : BoundaryNodes(false, false, lm),
    nodes {},
    cm_ (cm),
    is_normal_flow_ {false},
    beta1_ {},
    beta2_ {},
    beta3_ {}
{
  const auto c = lm_.GetLatticeSpeed();
  const auto cs_sqr = c * c / 3.0;
  beta1_ = c / cs_sqr / 9.0;
  beta2_ = 0.5 / c;
  beta3_ = beta2_ - beta1_;
}

void ZouHeNodes::AddNode(std::size_t x
  , std::size_t y
  , double u_x
  , double u_y)
{
  const auto nx = lm_.GetNumberOfColumns();
  const auto ny = lm_.GetNumberOfRows();
  const auto left = x == 0;
  const auto right = x == nx - 1;
  const auto bottom = y == 0;
  const auto top = y == ny - 1;
//  knowns_ = {!((left || right) && (top || bottom)),
//      right, top, left, bottom,
//      right || top, left || top, left || bottom, right || bottom};
//  auto n = y * nx + x;
  auto side = -1;
  if (right) side = 0;
  if (top) side = 1;
  if (left) side = 2;
  if (bottom) side = 3;
  // adds a corner node
  if ((top || bottom) && (left || right)) {
    side = right * 1 + top * 2;
    nodes.push_back(ValueNode(x, y, nx, u_x, u_y, true, side));
  }
  // adds a side node
  else {
    nodes.push_back(ValueNode(x, y, nx, u_x, u_y, false, side));
  }
}

void ZouHeNodes::UpdateNodes(std::vector<std::vector<double>> &df
  , bool is_modify_stream)
{
  if (!is_modify_stream) {
    for (auto node : nodes) {
      if (node.b1) {
        ZouHeNodes::UpdateCorner(df, node);
      }
      else {
        ZouHeNodes::UpdateSide(df, node);
      }
    }  // n
  }
}

void ZouHeNodes::UpdateSide(std::vector<std::vector<double>> &df
  , ValueNode &node)
{
  const auto n = node.n;
  const auto nx = lm_.GetNumberOfColumns();
  const auto c = lm_.GetLatticeSpeed();
  switch(node.i1) {
    case 0: {  // right
      auto vel = is_normal_flow_ ? lm_.u[n - 1] : node.v1;
      const auto rho_node = (df[n][0] + df[n][N] + df[n][S] + 2.0 * (df[n][E] +
          df[n][NE] + df[n][SE])) / (1.0 + vel[0] / c);
      const auto df_diff = 0.5 * (df[n][S] - df[n][N]);
      for (auto &u : vel) u *= rho_node;
      df[n][W] = df[n][E] - 2.0 * beta1_ * vel[0];
      df[n][NW] = df[n][SE] + df_diff - beta3_ * vel[0] + beta2_ * vel[1];
      df[n][SW] = df[n][NE] - df_diff - beta3_ * vel[0] - beta2_ * vel[1];
      break;
    }
    case 1: {  // top
      auto vel = is_normal_flow_ ? lm_.u[n - nx] : node.v1;
      const auto rho_node = (df[n][0] + df[n][E] + df[n][W] + 2.0 * (df[n][N] +
          df[n][NE] + df[n][NW])) / (1.0 + vel[1] / c);
      const auto df_diff = 0.5 * (df[n][E] - df[n][W]);
      for (auto &u : vel) u *= rho_node;
      df[n][S] = df[n][N] - 2.0 * beta1_ * vel[1];
      df[n][SW] = df[n][NE] + df_diff - beta2_ * vel[0] - beta3_ * vel[1];
      df[n][SE] = df[n][NW] - df_diff + beta2_ * vel[0] - beta3_ * vel[1];
      break;
    }
    case 2: {  // left
      auto vel = is_normal_flow_ ? lm_.u[n + 1] : node.v1;
      const auto rho_node = (df[n][0] + df[n][N] + df[n][S] + 2.0 * (df[n][W] +
          df[n][NW] + df[n][SW])) / (1.0 - vel[0] / c);
      const auto df_diff = 0.5 * (df[n][S] - df[n][N]);
      for (auto &u : vel) u *= rho_node;
      df[n][E] = df[n][W] + 2.0 * beta1_ * vel[0];
      df[n][NE] = df[n][SW] + df_diff + beta3_ * vel[0] + beta2_ * vel[1];
      df[n][SE] = df[n][NW] - df_diff + beta3_ * vel[0] - beta2_ * vel[1];
      break;
    }
    case 3: {  // bottom
      auto vel = is_normal_flow_ ? lm_.u[n + nx] : node.v1;
      const auto rho_node = (df[n][0] + df[n][E] + df[n][W] + 2.0 * (df[n][S] +
          df[n][SW] + df[n][SE])) / (1.0 - vel[1] / c);
      const auto df_diff = 0.5 * (df[n][W] - df[n][E]);
      for (auto &u : vel) u *= rho_node;
      df[n][N] = df[n][S] + 2.0 * beta1_ * vel[1];
      df[n][NE] = df[n][SW] + df_diff + beta2_ * vel[0] + beta3_ * vel[1];
      df[n][NW] = df[n][SE] - df_diff - beta2_ * vel[0] + beta3_ * vel[1];
      break;
    }
    default: {
      throw std::runtime_error("Not a side");
    }
  }
}

void ZouHeNodes::UpdateCorner(std::vector<std::vector<double>> &df
  , ValueNode &node)
{
  const auto n = node.n;
  auto vel = node.v1;
  const auto nx = lm_.GetNumberOfColumns();
  const auto nc = lm_.GetNumberOfDirections();
  switch (node.i1) {
    case 0: {  // bottom-left
      auto rho_node = 0.5 * (cm_.rho[n + nx] + cm_.rho[n + 1]);
      for (auto &u : vel) u *= rho_node;
      df[n][E] = df[n][W] + 2.0 * beta1_ * vel[0];
      df[n][N] = df[n][S] + 2.0 * beta1_ * vel[1];
      df[n][NE] = df[n][SW] + 0.5 * beta1_ * vel[0] + 0.5 * beta1_ * vel[1];
      df[n][NW] = -0.5 * beta3_ * vel[0] + 0.5 * beta3_ * vel[1];
      df[n][SE] = 0.5 * beta3_ * vel[0] - 0.5 * beta3_ * vel[1];
      for (auto i = 1u; i < nc; ++i) rho_node -= df[n][i];
      df[n][0] = rho_node;
      break;
    }
    case 1: {  // bottom-right
      auto rho_node = 0.5 * (cm_.rho[n + nx] + cm_.rho[n - 1]);
      for (auto &u : vel) u *= rho_node;
      df[n][W] = df[n][E] - 2.0 * beta1_ * vel[0];
      df[n][N] = df[n][S] + 2.0 * beta1_ * vel[1];
      df[n][NW] = df[n][SE] - 0.5 * beta1_ * vel[0] + 0.5 * beta1_ * vel[1];
      df[n][NE] = 0.5 * beta3_ * vel[0] + 0.5 * beta3_ * vel[1];
      df[n][SW] = -0.5 * beta3_ * vel[0] - 0.5 * beta3_ * vel[1];
      for (auto i = 1u; i < nc; ++i) rho_node -= df[n][i];
      df[n][0] = rho_node;
      break;
    }
    case 2: {  // top-left
      auto rho_node = 0.5 * (cm_.rho[n - nx] + cm_.rho[n + 1]);
      for (auto &u : vel) u *= rho_node;
      df[n][E] = df[n][W] + 2.0 * beta1_ * vel[0];
      df[n][S] = df[n][N] - 2.0 * beta1_ * vel[1];
      df[n][SE] = df[n][NW] + 0.5 * beta1_ * vel[0] - 0.5 * beta1_ * vel[1];
      df[n][NE] = 0.5 * beta3_ * vel[0] + 0.5 * beta3_ * vel[1];
      df[n][SW] = -0.5 * beta3_ * vel[0] - 0.5 * beta3_ * vel[1];
      for (auto i = 1u; i < nc; ++i) rho_node -= df[n][i];
      df[n][0] = rho_node;
      break;
    }
    case 3: {  // top-right
      auto rho_node = 0.5 * (cm_.rho[n - nx] + cm_.rho[n - 1]);
      for (auto &u : vel) u *= rho_node;
      df[n][W] = df[n][E] - 2.0 * beta1_ * vel[0];
      df[n][S] = df[n][N] - 2.0 * beta1_ * vel[1];
      df[n][SW] = df[n][NE] - 0.5 * beta1_ * vel[0] - 0.5 * beta1_ * vel[1];
      df[n][NW] = -0.5 * beta3_ * vel[0] + 0.5 * beta3_ * vel[1];
      df[n][SE] = 0.5 * beta3_ * vel[0] - 0.5 * beta3_ * vel[1];
      for (auto i = 1u; i < nc; ++i) rho_node -= df[n][i];
      df[n][0] = rho_node;
      break;
    }
    default: {
      throw std::runtime_error("Not a corner");
    }
  }
}

void ZouHeNodes::ToggleNormalFlow()
{
  is_normal_flow_ = true;
}
