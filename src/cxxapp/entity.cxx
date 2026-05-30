/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file entity.cxx
 * @author brian
 * @date 2024-08-31
 */

#include "entity.h"
#include "client/prediction.h"
#include "networkClass.h"
#include "networkObject.h"
#include "vectorNetClasses.h"
#include "gameGlobals.h"
#include "pdxElement.h"

#ifdef SERVER
#include "mapEntity.h"
#endif

NetworkClass *Entity::_network_class = nullptr;

/**
 *
 */
Entity::
Entity(const std::string &name) :
  _node_path("entity" + name),
  _name(name),
  _health(100),
  _max_health(100),
  _team(TFTEAM_no_team),
  _parent_entity(EPC_render),
  _view_offset(0.0f, 0.0f, 16.0f),
  _simulation_tick(-1)
{
#ifdef CLIENT
  _iv_pos = new InterpolatedVec3f;
  _iv_hpr = new InterpolatedVec3f;
  _iv_hpr->set_angles(true);
  make_interpolated_var<LVecBase3f>(_iv_pos, InterpVarFlags::IVF_simulation, nullptr, s_get_pos, s_set_pos);
  make_interpolated_var<LVecBase3f>(_iv_hpr, InterpVarFlags::IVF_simulation, nullptr, s_get_hpr, s_set_hpr);
#endif
}

/**
 *
 */
void Entity::
generate() {
  NetworkObject::generate();
  // TODO: entity parent.
  _node_path.reparent_to(globals.render);
}

/**
 *
 */
void Entity::
disable() {
#ifdef CLIENT
  shutdown_prediction();
#endif
  _node_path.remove_node();
  NetworkObject::disable();
}

/**
 *
 */
void Entity::
simulate() {
}

#ifdef SERVER

/**
 * Called when the entity is being created from the level.
 * The entity should read in property values from the props structure.
 */
void Entity::
init_from_level(const MapEntity *map_ent, PDXElement *props) {
  _class_name = map_ent->get_class_name();

  if (props->has_attribute("targetname")) {
    _target_name = props->get_attribute_value("targetname").get_string();
  }

  if (props->has_attribute("origin")) {
    LPoint3f origin;
    props->get_attribute_value("origin").to_vec3(origin);
    set_pos(origin);
  }

  if (props->has_attribute("angles")) {
    LVecBase3f angles;
    props->get_attribute_value("angles").to_vec3(angles);
    set_hpr(LVecBase3f(angles[1] - 90, -angles[0], angles[2]));
  }

  if (props->has_attribute("TeamNum")) {
    _team = props->get_attribute_value("TeamNum").get_int();
  }

  if (props->has_attribute("parentname")) {
    // We just store it when initializing.  We will resolve the parent
    // and actually reparent the entity later in generate().
    _parent_entity_target_name = props->get_attribute_value("parentname").get_string();
  }
}

#endif

#ifdef CLIENT

/**
 *
 */
void Entity::
post_data_update() {
  NetworkObject::post_data_update();
  bool predict = should_predict();
  if (predict && _pred == nullptr) {
    // Entity should be predicted and we haven't initialized yet.
    init_prediction();
  } else if (!predict && _pred != nullptr) {
    // No longer predicting this entity.
    shutdown_prediction();
  }
}

/**
 *
 */
bool Entity::
should_predict() const {
  return false;
}

/**
 *
 */
void Entity::
init_prediction() {
  if (_pred != nullptr) {
    return;
  }

  _pred = new PredictedObject;
  _pred->entity = this;

  // Setup prediction fields.
  add_prediction_fields();

  _pred->calc_sizes();
  _pred->post_networked_data_received(0, 0);
  // Now fill everything.
  for (int i = 0; i < prediction_num_data_slots; ++i) {
    _pred->save_data(i, PredictionCopy::CM_everything);
  }

  _predictable = true;
  Prediction::ptr()->add_predictable(_pred);
}

/**
 *
 */
void Entity::
shutdown_prediction() {
  if (_pred == nullptr) {
    return;
  }

  _predictable = false;
  Prediction::ptr()->remove_predictable(_pred);
  _pred = nullptr;
}

/**
 *
 */
void Entity::
add_prediction_fields() {
  auto pos_field = add_pred_field<LVecBase3f>("pos", nullptr, s_set_pos, s_get_pos);
  pos_field->tolerance = 0.02f;
  auto hpr_field = add_pred_field<LVecBase3f>("hpr", nullptr, s_set_hpr, s_get_hpr);
  hpr_field->no_error_check = true;
}

#endif

/**
 *
 */
void Entity::
s_set_pos(LVecBase3f pos, void *pent) {
  Entity *ent = (Entity *)pent;
  ent->set_pos(pos);
}

/**
 *
 */
LVecBase3f Entity::
s_get_pos(void *pent) {
  return ((Entity *)pent)->get_pos();
}

/**
 *
 */
void Entity::
s_set_hpr(LVecBase3f hpr, void *pent) {
  Entity *ent = (Entity *)pent;
  ent->set_hpr(hpr);
}

/**
 *
 */
LVecBase3f Entity::
s_get_hpr(void *pent) {
  Entity *e = (Entity *)pent;
  return e->get_hpr();
}

/**
 *
 */
static void
fetch_entity_pos(void *object, void *dest) {
  Entity *e = (Entity *)object;
  LPoint3f pos = e->get_pos();
  *(LPoint3f *)dest = pos;
}

/**
 *
 */
static void
write_entity_pos(void *object, void *data) {
  LPoint3f pos = *(LPoint3f *)data;
  Entity *e = (Entity *)object;
  e->set_pos(pos);
}

/**
 *
 */
static void
fetch_entity_hpr(void *object, void *dest) {
  Entity *e = (Entity *)object;
  LVecBase3f hpr = e->get_hpr();
  *(LVecBase3f *)dest = hpr;
}

/**
 *
 */
static void
write_entity_hpr(void *object, void *data) {
  Entity *e = (Entity *)object;
  LVecBase3f hpr = *(LVecBase3f *)data;
  e->set_hpr(hpr);
}

void Entity::
init_network_class() {
  BEGIN_NETWORK_CLASS_NOBASE(Entity);

    MAKE_NET_FIELD(Entity, _health, NetworkField::DT_int16);
    MAKE_NET_FIELD(Entity, _max_health, NetworkField::DT_int16);
    MAKE_NET_FIELD(Entity, _parent_entity, NetworkField::DT_int16);
    MAKE_NET_FIELD(Entity, _team, NetworkField::DT_int8);
    MAKE_INDIRECT_STRUCT_NET_FIELD(LPoint3f, pos, Position_NetClass::get_network_class(), fetch_entity_pos, write_entity_pos);
    MAKE_INDIRECT_STRUCT_NET_FIELD(LVecBase3f, hpr, Angles_NetClass::get_network_class(), fetch_entity_hpr, write_entity_hpr);

  END_NETWORK_CLASS();
}
