extends Control


# Declare member variables here. Examples:
# var a = 2
# var b = "text"

export var fps_label_path: NodePath
export var player_label_path: NodePath
export var ai_start_button_path: NodePath
export var ai_controls_container_path: NodePath
export var ai_stats_container_path: NodePath
export var total_time_label_path: NodePath
export var game_time_label_path: NodePath
export var generation_label_path: NodePath
export var lives_label_path: NodePath
export var ai_id_label_path: NodePath
export var score_label_path: NodePath

# externally changed variables
var is_player: bool = false
var elapsed_match_time: float = 0
var generation: int = 0
var lives: int = 0
var ai_id: int = 0
var score: float = 0

# "private" variables
var fps_label: Label
var player_label: Label
var ai_start_button: Button
var ai_controls_container: Container
var ai_stats_container: Container
var total_time_label: Label
var game_time_label: Label
var generation_label: Label
var lives_label: Label
var ai_id_label: Label
var score_label: Label

var total_elapsed: float = 0


# Called when the node enters the scene tree for the first time.
func _ready():
    fps_label = get_node(fps_label_path)
    player_label = get_node(player_label_path)
    ai_start_button = get_node(ai_start_button_path)
    ai_controls_container = get_node(ai_controls_container_path)
    ai_stats_container = get_node(ai_stats_container_path)
    total_time_label = get_node(total_time_label_path)
    game_time_label = get_node(game_time_label_path)
    generation_label = get_node(generation_label_path)
    lives_label = get_node(lives_label_path)
    ai_id_label = get_node(ai_id_label_path)
    score_label = get_node(score_label_path)


# Called from C++ when match startup code has ran
func on_start():
    apply_visibility()


func _process(delta):
    # We keep track of total run time (if we ever move to having the AI be
    # paused at the start, this needs changes)
    total_elapsed += delta

    fps_label.text = "FPS: %s" % Engine.get_frames_per_second()

    total_time_label.text = "Total Time: %s" % int(total_elapsed)
    game_time_label.text = "Game Time: %s" % stepify(elapsed_match_time, 0.1)
    score_label.text = "Score: %s" % stepify(score, 0.1)
    lives_label.text = "Lives: %s" % lives

    if is_player:
        player_label.text = "Player: user"
        generation_label.text = "Generation: %s" % generation
        ai_id_label.text = "Watching AI: %s" % ai_id
    else:
        player_label.text = "Player: AI"

func apply_visibility():
    # ai_start_button.visible = !is_player
    # For now the AI autostarts so this button is never used
    ai_start_button.visible = false
    ai_controls_container.visible = !is_player
    ai_stats_container.visible = !is_player


func _on_QuitButton_pressed():
    if get_tree().change_scene("res://src/MainMenu.tscn") != OK:
        printerr("failed to load main menu scene")

