#ifdef SERVER

#include "server.h"
#include "configVariableInt.h"
#include "configVariableString.h"
#include "../networkClasses.h"
#include "../gameManager.h"
#include "../gameGlobals.h"
#include "asyncTaskManager.h"
#include "config_anim.h"
#include "clockObject.h"
#include "physSystem.h"
#include "physScene.h"
#include "../gamePhysics.h"
#include "jobSystem.h"

ConfigVariableInt sv_port("sv-port", "27015", PRC_DESC("Server port to talk over."));
ConfigVariableString tf_map("tf-map", "levels/ctf_2fort", PRC_DESC("Level to load."));

/**
 * Server entry point.
 */
int
main(int argc, char *argv[]) {
  init_libanim();
  JobSystem::init_global_job_system();

  init_network_classes();

  globals.render = NodePath("render");
  globals.dyn_render = globals.render.attach_new_node("dynamic_render");

  GamePhysics *phys = GamePhysics::ptr();
  phys->initialize();

  GameServer *sv = GameServer::ptr();

  globals.simbase = sv;
  globals.sv = sv;
  globals.task_mgr = AsyncTaskManager::get_global_ptr();
  PT(AsyncTaskManager) sim_task_mgr = new AsyncTaskManager("simulation");
  globals.sim_task_mgr = sim_task_mgr;

  sv->startup(sv_port);

  PT(GameManager) game_mgr = new GameManager;
  sv->generate_object(game_mgr, game_manager_zone);
  // Load the level.
  game_mgr->change_level(tf_map.get_value());

  ClockObject *clock = ClockObject::get_global_clock();

  while (true) {
    clock->tick();
    sv->run_frame();
    phys->update(clock->get_frame_time());
  }

  return 0;
}

#endif // SERVER
