#ifndef TFNODEDATA_H
#define TFNODEDATA_H

#include "typedReferenceCount.h"
#include "nodePath.h"
#include "pandaNode.h"

class Entity;

/**
 * Data we can attach to nodes.  We typically attach them to physics
 * nodes so when we run a query and get back a physics node
 * we can figure out which entity it belongs to, etc.
 */
class TFNodeData : public TypedReferenceCount {
public:
  Entity *entity = nullptr;
  void *object = nullptr;
  int prop_index = -1;
  int hit_box = -1;

  inline static TFNodeData *get(const NodePath &np);
  inline static TFNodeData *get(PandaNode *node);
  inline static TFNodeData *get_or_create(const NodePath &np);
  inline static TFNodeData *get_or_create(PandaNode *node);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "TFNodeData",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return _type_handle;
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

/**
 *
 */
inline TFNodeData *TFNodeData::
get(const NodePath &np) {
  return get(np.node());
}

/**
 *
 */
inline TFNodeData *TFNodeData::
get(PandaNode *node) {
  return node->get_c_tag<TFNodeData>();
}

/**
 *
 */
inline TFNodeData *TFNodeData::
get_or_create(const NodePath &np) {
  return get_or_create(np.node());
}

/**
 *
 */
inline TFNodeData *TFNodeData::
get_or_create(PandaNode *node) {
  PT(TFNodeData) node_data = node->get_c_tag<TFNodeData>();
  if (node_data == nullptr) {
    node_data = new TFNodeData;
    node->set_c_tag<TFNodeData>(node_data);
  }
  return node_data;
}

#endif // TFNODEDATA_H
