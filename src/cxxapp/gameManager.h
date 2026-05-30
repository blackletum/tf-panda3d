#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "networkObject.h"
#include "nodePath.h"
#include "mapData.h"
#include "pointerTo.h"
#include "pvector.h"
#include "entity.h"

/**
 *
 */
class GameManager : public NetworkObject {
public:
  virtual void generate() override;
  virtual void disable() override;

  inline void set_level_name(const std::string &name);
  inline const std::string &get_level_name() const;

  inline MapData *get_map_data() const;

  inline const NodePath &get_prop_phys_root() const;
  inline NodePathCollection get_prop_physics_nodes() const;

  void change_level(const Filename &level_filename);
  void unload_level();

private:
  void load_level_props();
  bool r_process_prop_node(PandaNode *node, const MapStaticProp *sprop);
  bool process_prop_geom_node(GeomNode *node, const MapStaticProp *sprop);

#ifdef SERVER
  void load_level_entities();
#endif

  const GeomVertexArrayFormat *get_baked_vertex_lighting_array_format();
  const RenderState *get_baked_vertex_lighting_state();

  static void s_recv_join_game(void *obj, void *args);
  static void s_recv_join_game_resp(void *obj, void *args);

private:
  std::string _game_name;
  std::string _level_name;
  NodePath _level_path;
  PT(MapData) _level_data;
  NodePath _prop_root;
  int _geom_index;
  NodePathCollection _prop_models;
  NodePath _prop_phys_root;

  CPT(GeomVertexArrayFormat) _baked_lighting_array_format;
  CPT(RenderState) _baked_vertex_lighting_state;

#ifdef SERVER
  // Entities that we loaded from the level file.
  typedef pvector<PT(Entity)> EntityList;
  EntityList _level_entities;
#endif

public:
  static NetworkObject *make_GameManager() {
    return new GameManager;
  }
  virtual NetworkClass *get_network_class() const override {
    return _network_class;
  }
  static NetworkClass *get_type_network_class() {
    return _network_class;
  }
  static void init_network_class();
private:
  static NetworkClass *_network_class;
};

/**
 *
 */
inline void GameManager::
set_level_name(const std::string &name) {
  _level_name = name;
}

/**
 *
 */
inline const std::string &GameManager::
get_level_name() const {
  return _level_name;
}

/**
 *
 */
inline MapData *GameManager::
get_map_data() const {
  return _level_data;
}

/**
 *
 */
inline const NodePath &GameManager::
get_prop_phys_root() const {
  return _prop_phys_root;
}

/**
 *
 */
inline NodePathCollection GameManager::
get_prop_physics_nodes() const {
  return _prop_phys_root.find_all_matches("**/+PhysRigidActorNode");
}

#endif // GAMEMANAGER_H
