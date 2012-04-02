/*
 * Copyright 2011 Nate Koenig & Andrew Howard
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
/* Desc: Axis Visualization Class
 * Author: Nate Koenig
 */

#include "rendering/ogre.h"
#include "rendering/Scene.hh"
#include "rendering/ArrowVisual.hh"
#include "rendering/AxisVisual.hh"

using namespace gazebo;
using namespace rendering;

/////////////////////////////////////////////////
AxisVisual::AxisVisual(const std::string &_name, VisualPtr _vis)
  : Visual(_name, _vis)
{
}

/////////////////////////////////////////////////
AxisVisual::~AxisVisual()
{
  this->xAxis.reset();
  this->yAxis.reset();
  this->zAxis.reset();
}

/////////////////////////////////////////////////
void AxisVisual::Load()
{
  printf("AxisVisual::Load\n");
  Visual::Load();

  this->xAxis.reset(new ArrowVisual(this->GetName() +
      "_X_AXIS", shared_from_this()));
  this->xAxis->Load();
  this->xAxis->SetMaterial("__GAZEBO_TRANS_RED_MATERIAL__");

  this->yAxis.reset(new ArrowVisual(this->GetName() +
      "_Y_AXIS", shared_from_this()));
  this->yAxis->Load();
  this->xAxis->SetMaterial("__GAZEBO_TRANS_GREEN_MATERIAL__");

  this->zAxis.reset(new ArrowVisual(this->GetName() +
      "_Z_AXIS", shared_from_this()));
  this->zAxis->Load();
  this->xAxis->SetMaterial("__GAZEBO_TRANS_BLUE_MATERIAL__");

  double length = 0.2;
  this->xAxis->SetPosition(math::Vector3(length/2.0, 0.0, 0.0));
  this->xAxis->SetRotation(
      math::Quaternion(math::Vector3(0, 0, 1), GZ_DTOR(-90)));

  this->yAxis->SetPosition(math::Vector3(0.0, length/2.0, 0.0));

  this->zAxis->SetPosition(math::Vector3(0.0, 0.0, length/2.0));
  this->zAxis->SetRotation(
      math::Quaternion(math::Vector3(1, 0 ,0), GZ_DTOR(90)));

  this->SetVisibilityFlags(GZ_VISIBILITY_GUI);
}
