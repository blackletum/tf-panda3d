/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file world.cxx
 * @author brian
 * @date 2024-09-01
 */

#include "world.h"
#include "mapData.h"
#include "physTriangleMeshData.h"
#include "physTriangleMesh.h"
#include "physMaterial.h"
#include "physShape.h"
#include "physRigidStaticNode.h"
#include "nodePath.h"
#include "geomNode.h"
#include "networkClass.h"
#include "gameGlobals.h"
#include "gamePhysics.h"
#include "gameManager.h"
#include "tfNodeData.h"
#ifdef CLIENT
#include "client/indexBufferCombiner.h"
#endif
#ifdef SERVER
#include "entityFactory.h"
#endif

NetworkClass *World::_network_class;

/**
 *
 */
World::
World() :
  Entity("world"),
  _model_index(0),
  _map_model(nullptr)
{
}

/**
 *
 */
void World::
generate() {
  globals.world = this;

  Entity::generate();

  _map_model = globals.game->get_map_data()->get_model(_model_index);
  init_world_collisions();
  parent_model_geometry();

  get_node_path().reparent_to(globals.render);

  // Link static prop physics to world entity.
  NodePathCollection prop_physics_nodes = globals.game->get_prop_physics_nodes();
  for (int i = 0; i < prop_physics_nodes.get_num_paths(); ++i) {
    TFNodeData *node_data = TFNodeData::get_or_create(prop_physics_nodes.get_path(i));
    node_data->entity = this;
    node_data->object = this;
  }

  get_node_path().flatten_light();

#ifdef CLIENT
  // Share index buffers across all the world geoms.
  {
    IndexBufferCombiner ibc(get_node_path());
    ibc.combine();
  }
#endif

  globals.render.ls();
}

/**
 * Parents the level geometry associated with this entity to the entity itself,
 * so it appears in the scene.
 */
void World::
parent_model_geometry() {
  GeomNode *gn = _map_model->get_geom_node();
  if (gn != nullptr) {
    NodePath np(gn->make_copy());
    np.flatten_light();
    np.reparent_to(get_node_path());
  }
}

/**
 *
 */
static CollideMask
get_collide_mask_from_type(const std::string &type) {
  if (type == "clip" || type == "playerclip") {
    return CollideMask_player_clip;
  } else if (type == "sky") {
    return CollideMask_sky;
  } else {
    return CollideMask_world;
  }
}

/**
 *
 */
void World::
init_world_collisions() {
  // Load collisions from world map data.

  GamePhysics *phys = GamePhysics::ptr();

  for (int i = 0; i < _map_model->get_num_tri_groups(); ++i) {
    const MapModel::CollisionGroup *group = _map_model->get_tri_group(i);
    PhysTriangleMesh tri_mesh(new PhysTriangleMeshData(group->get_tri_mesh_data()));
    pvector<PT(PhysMaterial)> materials;
    for (int j = 0; j < group->get_num_surface_props(); ++j) {
      materials.push_back(phys->get_surface_material(downcase(group->get_surface_prop(j))));
    }

    assert(materials.size() > 0u);

    PT(PhysShape) shape = new PhysShape(tri_mesh, materials[0]);
    shape->set_scene_query_shape(true);
    shape->set_simulation_shape(true);
    shape->set_trigger_shape(false);

    // Append remaining materials if there are multiple.
    if (materials.size() > 1u) {
      for (size_t j = 1u; j < materials.size(); ++j) {
        shape->add_material(materials[i]);
      }
      shape->submit_materials();
    }

    PT(PhysRigidStaticNode) body = new PhysRigidStaticNode("world-collide-" + group->get_collide_type());
    body->set_from_collide_mask(get_collide_mask_from_type(group->get_collide_type()));
    body->add_shape(shape);
    body->add_to_scene(phys->get_phys_world());
    // Link the collision node up with our entity.
    TFNodeData *node_data = TFNodeData::get_or_create(body);
    node_data->entity = this;
    node_data->object = this;

    // Put it under our entity node in the SG.
    get_node_path().attach_new_node(body);

    _world_collisions.push_back(body);
  }
}

/**
 *
 */
void World::
disable() {
  // Tear down the static world collisions.
  for (PhysRigidStaticNode *body : _world_collisions) {
    body->remove_from_scene(globals.physics_world);
  }
  _world_collisions.clear();
  _map_model = nullptr;
  globals.world = nullptr;

  Entity::disable();
}

#ifdef SERVER
/**
 *
 */
void World::
register_ent_factory()  {
  EntityFactory::get_global_ptr()->register_entity("worldspawn", create_ent_World);
}
#endif

/**
 *
 */
void World::
init_network_class() {
  BEGIN_NETWORK_CLASS(World, Entity);
  _network_class->set_factory_func(create_World);
  MAKE_NET_FIELD(World, _model_index, NetworkField::DT_int32);
  END_NETWORK_CLASS();
}
