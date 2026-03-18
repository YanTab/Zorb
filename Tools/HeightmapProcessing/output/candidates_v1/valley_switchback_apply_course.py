import json
import unreal

LAYOUT_JSON = r"D:/Game_Project/ZorbEarthRun/Tools/HeightmapProcessing/output/candidates_v1/valley_switchback_course.json"

CHECKPOINT_BP = "/Game/Blueprints/BP_CheckPointTrigger.BP_CheckPointTrigger_C"
FINISH_BP = "/Game/Blueprints/BP_FinishTrigger.BP_FinishTrigger_C"


def _vec(d):
    return unreal.Vector(float(d["x"]), float(d["y"]), float(d["z"]))


def _rot(d):
    return unreal.Rotator(float(d["pitch"]), float(d["yaw"]), float(d["roll"]))


def _load_class(path):
    c = unreal.load_class(None, path)
    if c is None:
        raise RuntimeError(f"Cannot load class: {path}")
    return c


def _move_or_spawn_player_start(start_loc, start_rot):
    world = unreal.EditorLevelLibrary.get_editor_world()
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in actors:
        if isinstance(actor, unreal.PlayerStart):
            actor.set_actor_location(start_loc, False, False)
            actor.set_actor_rotation(start_rot, False)
            actor.set_actor_label("AUTO_PlayerStart")
            return actor

    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PlayerStart, start_loc, start_rot)
    actor.set_actor_label("AUTO_PlayerStart")
    return actor


def _delete_previous_auto_markers():
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    to_delete = []
    for actor in actors:
        label = actor.get_actor_label()
        if label.startswith("AUTO_CP_") or label == "AUTO_Finish":
            to_delete.append(actor)
    for actor in to_delete:
        unreal.EditorLevelLibrary.destroy_actor(actor)


def main():
    with open(LAYOUT_JSON, "r", encoding="utf-8") as f:
        data = json.load(f)

    cp_class = _load_class(CHECKPOINT_BP)
    finish_class = _load_class(FINISH_BP)

    _delete_previous_auto_markers()

    start_loc = _vec(data["start"]["location"])
    start_rot = _rot(data["start"]["rotation"])
    _move_or_spawn_player_start(start_loc, start_rot)

    for cp in data["checkpoints"]:
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            cp_class,
            _vec(cp["location"]),
            _rot(cp["rotation"]),
        )
        actor.set_actor_label(f"AUTO_CP_{int(cp['index']):02d}")

    finish = data["finish"]
    finish_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        finish_class,
        _vec(finish["location"]),
        _rot(finish["rotation"]),
    )
    finish_actor.set_actor_label("AUTO_Finish")

    unreal.log(f"Auto course generated for map: {data.get('map_name', 'unknown')}")
    unreal.log(f"Checkpoints count: {len(data.get('checkpoints', []))}")


main()
