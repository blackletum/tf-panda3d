/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file world.h
 * @author brian
 * @date 2024-09-01
 */

#ifndef WORLD_H
#define WORLD_H

#include "entity.h"
#include "pointerTo.h"
#include "physRigidStaticNode.h"
#include "pvector.h"

class MapModel;

class World : public Entity {
public:
  World();

  virtual void generate() override;
  virtual void disable() override;

private:
  void init_world_collisions();
  void parent_model_geometry();

private:
  typedef pvector<PT(PhysRigidStaticNode)> WorldCollisionList;
  WorldCollisionList _world_collisions;

  int _model_index;
  const MapModel *_map_model;

public:
#ifdef SERVER
  static Entity *create_ent_World() {
    return new World;
  }
  static void register_ent_factory();
#endif
  static NetworkObject *create_World() {
    return new World;
  }
  static NetworkClass *get_type_network_class() {
    return _network_class;
  }
  virtual NetworkClass *get_network_class() const override {
    return _network_class;
  }
  static void init_network_class();
private:
  static NetworkClass *_network_class;
};

#endif // WORLD_H
