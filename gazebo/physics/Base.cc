/*
 * Copyright (C) 2012-2016 Open Source Robotics Foundation
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
// We also include this winsock2 trick in Base.hh but it is used last,
// so we need it again here.
#ifdef _WIN32
  // Ensure that Winsock2.h is included before Windows.h, which can get
  // pulled in by anybody (e.g., Boost).
  #include <Winsock2.h>
#endif

#include "gazebo/common/Assert.hh"
#include "gazebo/common/Console.hh"
#include "gazebo/common/Exception.hh"
#include "gazebo/physics/PhysicsIface.hh"
#include "gazebo/physics/World.hh"

#include "gazebo/physics/BasePrivate.hh"
#include "gazebo/physics/Base.hh"

using namespace gazebo;
using namespace physics;

//////////////////////////////////////////////////
Base::Base(BasePtr _parent)
: baseDPtr(new BasePrivate)
{
  this->baseDPtr->parent = _parent;
  this->ConstructionHelper();
}

//////////////////////////////////////////////////
Base::Base(BasePrivate &_dataPtr, BasePtr _parent)
: baseDPtr(&_dataPtr)
{
  this->baseDPtr->parent = _parent;
  this->ConstructionHelper();
}

//////////////////////////////////////////////////
void Base::ConstructionHelper()
{
  this->baseDPtr->type = BASE;
  this->baseDPtr->id = physics::getUniqueId();
  this->baseDPtr->saveable = true;
  this->baseDPtr->selected = false;

  this->baseDPtr->sdf.reset(new sdf::Element);
  this->baseDPtr->sdf->AddAttribute("name", "string", "__default__", true);
  this->baseDPtr->name = "__default__";

  if (this->baseDPtr->parent)
  {
    this->baseDPtr->world = this->baseDPtr->parent->World();
  }
}

//////////////////////////////////////////////////
Base::~Base()
{
  // remove self as a child of the parent
  if (this->baseDPtr->parent)
    this->baseDPtr->parent->RemoveChild(this->baseDPtr->id);

  this->SetParent(BasePtr());

  for (Base_V::iterator iter = this->baseDPtr->children.begin();
       iter != this->baseDPtr->children.end(); ++iter)
  {
    if (*iter)
      (*iter)->SetParent(BasePtr());
  }
  this->baseDPtr->children.clear();
  if (this->baseDPtr->sdf)
    this->baseDPtr->sdf->Reset();
  this->baseDPtr->sdf.reset();

  delete this->baseDPtr;
  this->baseDPtr = NULL;
}

//////////////////////////////////////////////////
void Base::Load(sdf::ElementPtr _sdf)
{
  if (_sdf)
    this->baseDPtr->sdf = _sdf;

  GZ_ASSERT(this->baseDPtr->sdf != NULL, "this->baseDPtr->sdf is NULL");

  if (this->baseDPtr->sdf->HasAttribute("name"))
    this->baseDPtr->name = this->baseDPtr->sdf->Get<std::string>("name");
  else
    this->baseDPtr->name.clear();

  if (this->baseDPtr->parent)
  {
    this->baseDPtr->world = this->baseDPtr->parent->World();
    this->baseDPtr->parent->AddChild(shared_from_this());
  }

  this->baseDPtr->ComputeScopedName();
}

//////////////////////////////////////////////////
void Base::UpdateParameters(sdf::ElementPtr _sdf)
{
  GZ_ASSERT(_sdf != NULL, "_sdf parameter is NULL");
  GZ_ASSERT(this->baseDPtr->sdf != NULL, "Base sdf member is NULL");
  this->baseDPtr->sdf->Copy(_sdf);
}

//////////////////////////////////////////////////
void Base::Fini()
{
  Base_V::iterator iter;

  for (iter = this->baseDPtr->children.begin(); iter != this->baseDPtr->children.end(); ++iter)
    if (*iter)
      (*iter)->Fini();

  this->baseDPtr->children.clear();

  this->baseDPtr->world.reset();
  this->baseDPtr->parent.reset();
}

//////////////////////////////////////////////////
void Base::Reset()
{
}

//////////////////////////////////////////////////
void Base::Reset(Base::EntityType _resetType)
{
  Base_V::iterator iter;
  for (iter = this->baseDPtr->children.begin(); iter != this->baseDPtr->children.end(); ++iter)
  {
    if ((*iter)->HasType(_resetType))
      (*iter)->Reset();

    (*iter)->Reset(_resetType);
  }
}

//////////////////////////////////////////////////
void Base::SetName(const std::string &_name)
{
  GZ_ASSERT(this->baseDPtr->sdf != NULL, "Base sdf member is NULL");
  GZ_ASSERT(this->baseDPtr->sdf->GetAttribute("name"),
      "Base sdf missing name attribute");
  this->baseDPtr->sdf->GetAttribute("name")->Set(_name);
  this->baseDPtr->name = _name;
  this->baseDPtr->ComputeScopedName();
}

//////////////////////////////////////////////////
std::string Base::GetName() const
{
  return this->Name();
}

//////////////////////////////////////////////////
std::string Base::Name() const
{
  return this->baseDPtr->name;
}

//////////////////////////////////////////////////
uint32_t Base::GetId() const
{
  return this->Id();
}

//////////////////////////////////////////////////
uint32_t Base::Id() const
{
  return this->baseDPtr->id;
}

//////////////////////////////////////////////////
void Base::SetSaveable(const bool _v)
{
  this->baseDPtr->saveable = _v;
}

//////////////////////////////////////////////////
bool Base::GetSaveable() const
{
  return this->Saveable();
}

//////////////////////////////////////////////////
bool Base::Saveable() const
{
  return this->baseDPtr->saveable;
}

//////////////////////////////////////////////////
int Base::GetParentId() const
{
  return this->ParentId();
}

//////////////////////////////////////////////////
int Base::ParentId() const
{
  return this->baseDPtr->parent == NULL ? 0 : this->baseDPtr->parent->Id();
}

//////////////////////////////////////////////////
void Base::SetParent(BasePtr _parent)
{
  this->baseDPtr->parent = _parent;
}

//////////////////////////////////////////////////
BasePtr Base::GetParent() const
{
  return this->Parent();
}

//////////////////////////////////////////////////
BasePtr Base::Parent() const
{
  return this->baseDPtr->parent;
}

//////////////////////////////////////////////////
void Base::AddChild(BasePtr _child)
{
  if (_child == NULL)
    gzthrow("Cannot add a null _child to an entity");

  // Add this _child to our list
  this->baseDPtr->children.push_back(_child);
}

//////////////////////////////////////////////////
void Base::RemoveChild(unsigned int _id)
{
  Base_V::iterator iter;
  for (iter = this->baseDPtr->children.begin();
       iter != this->baseDPtr->children.end(); ++iter)
  {
    if ((*iter)->Id() == _id)
    {
      (*iter)->Fini();
      this->baseDPtr->children.erase(iter);
      break;
    }
  }
}

//////////////////////////////////////////////////
unsigned int Base::GetChildCount() const
{
  return this->ChildCount();
}

//////////////////////////////////////////////////
unsigned int Base::ChildCount() const
{
  return this->baseDPtr->children.size();
}

//////////////////////////////////////////////////
void Base::AddType(Base::EntityType _t)
{
  this->baseDPtr->type = this->baseDPtr->type | (unsigned int)_t;
}

//////////////////////////////////////////////////
BasePtr Base::GetChild(unsigned int _i) const
{
  return this->Child(_i);
}

//////////////////////////////////////////////////
BasePtr Base::Child(const unsigned int _i) const
{
  if (_i < this->baseDPtr->children.size())
    return this->baseDPtr->children[_i];

  return BasePtr();
}

//////////////////////////////////////////////////
BasePtr Base::GetChild(const std::string &_name)
{
  return this->Child(_name);
}

//////////////////////////////////////////////////
BasePtr Base::Child(const std::string &_name) const
{
  std::string fullName = this->ScopedName() + "::" + _name;
  return BasePtr(this->BaseByName(fullName));
}

//////////////////////////////////////////////////
void Base::RemoveChild(const std::string &_name)
{
  Base_V::iterator iter;

  for (iter = this->baseDPtr->children.begin();
      iter != this->baseDPtr->children.end(); ++iter)
  {
    if ((*iter)->ScopedName() == _name)
      break;
  }

  if (iter != this->baseDPtr->children.end())
  {
    (*iter)->Fini();
    this->baseDPtr->children.erase(iter);
  }
}

//////////////////////////////////////////////////
void Base::RemoveChildren()
{
  this->baseDPtr->children.clear();
}

//////////////////////////////////////////////////
BasePtr Base::GetById(unsigned int _id) const
{
  return this->BaseById(_id);
}

//////////////////////////////////////////////////
BasePtr Base::BaseById(const unsigned int _id) const
{
  BasePtr result;
  Base_V::const_iterator biter;

  for (biter = this->baseDPtr->children.begin();
       biter != this->baseDPtr->children.end(); ++biter)
  {
    if ((*biter)->Id() == _id)
    {
      result = *biter;
      break;
    }
  }

  return result;
}

//////////////////////////////////////////////////
BasePtr Base::GetByName(const std::string &_name)
{
  return BasePtr(this->BaseByName(_name));
}

//////////////////////////////////////////////////
Base *Base::BaseByName(const std::string &_name) const
{
  if (this->ScopedName() == _name || this->Name() == _name)
    return const_cast<Base*>(this);

  Base *result;

  for (auto iter = this->baseDPtr->children.begin();
       iter != this->baseDPtr->children.end() && result == NULL; ++iter)
  {
    result = (*iter)->BaseByName(_name);
  }

  return result;
}

//////////////////////////////////////////////////
std::string Base::GetScopedName(bool _prependWorldName) const
{
  return this->ScopedName(_prependWorldName);
}

//////////////////////////////////////////////////
std::string Base::ScopedName(bool _prependWorldName) const
{
  if (_prependWorldName && this->baseDPtr->world)
    return this->baseDPtr->world->GetName() + "::" + this->baseDPtr->scopedName;
  else
    return this->baseDPtr->scopedName;
}

//////////////////////////////////////////////////
bool Base::HasType(const Base::EntityType &_t) const
{
  return ((unsigned int)(_t & this->baseDPtr->type) == (unsigned int)_t);
}

//////////////////////////////////////////////////
unsigned int Base::GetType() const
{
  return this->Type();
}

//////////////////////////////////////////////////
unsigned int Base::Type() const
{
  return this->baseDPtr->type;
}

//////////////////////////////////////////////////
void Base::Print(const std::string &_prefix)
{
  Base_V::iterator iter;
  gzmsg << _prefix << this->Name() << "\n";

  for (iter = this->baseDPtr->children.begin();
       iter != this->baseDPtr->children.end(); ++iter)
  {
    (*iter)->Print(_prefix + "  ");
  }
}

//////////////////////////////////////////////////
bool Base::SetSelected(const bool _s)
{
  this->baseDPtr->selected = _s;

  Base_V::iterator iter;
  for (iter = this->baseDPtr->children.begin(); iter != this->baseDPtr->children.end(); ++iter)
    (*iter)->SetSelected(_s);

  return true;
}

//////////////////////////////////////////////////
bool Base::IsSelected() const
{
  return this->baseDPtr->selected;
}

//////////////////////////////////////////////////
bool Base::operator ==(const Base &_ent) const
{
  return _ent.Name() == this->Name();
}

//////////////////////////////////////////////////
void Base::SetWorld(const WorldPtr &_newWorld)
{
  this->baseDPtr->world = _newWorld;

  Base_V::iterator iter;
  for (iter = this->baseDPtr->children.begin();
       iter != this->baseDPtr->children.end(); ++iter)
  {
    (*iter)->SetWorld(this->baseDPtr->world);
  }
}

//////////////////////////////////////////////////
const WorldPtr &Base::GetWorld() const
{
  return this->World();
}

//////////////////////////////////////////////////
const WorldPtr &Base::World() const
{
  return this->baseDPtr->world;
}

//////////////////////////////////////////////////
const sdf::ElementPtr Base::SDF() const
{
  GZ_ASSERT(this->baseDPtr->sdf != NULL, "Base sdf member is NULL");
  this->baseDPtr->sdf->Update();
  return this->baseDPtr->sdf;
}

//////////////////////////////////////////////////
void BasePrivate::ComputeScopedName()
{
  BasePtr p = this->parent;
  this->scopedName = this->name;

  while (p)
  {
    if (p->Parent())
      this->scopedName.insert(0, p->Name()+"::");
    p = p->Parent();
  }
}


