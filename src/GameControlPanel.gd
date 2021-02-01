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
export var alive_ais_label_path: NodePath
export var ai_performance_label_path: NodePath
export var update_performance_label_path: NodePath
export var speed_control_path: NodePath
export var thread_count_label_path: NodePath
export var thread_slider_path: NodePath
export var status_label_path: NodePath
export var save_ai_button_path: NodePath
export var ball_speed_label_path: NodePath
export var ball_speed_slider_path: NodePath
export var paddle_speed_label_path: NodePath
export var paddle_speed_slider_path: NodePath

export var status_hide_time: float = 3

signal training_speed_changed
signal threads_changed
signal save_ai_pressed
signal paddle_speed_changed
signal ball_speed_changed

# externally changed variables
var is_player: bool = false
var elapsed_match_time: float = 0
var generation: int = 0
var lives: int = 0
var ai_id: int = 0
var score: float = 0
var alive_ais: int = 0
var update_performance: float = 0
var ai_performance: float = 0


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
var alive_ais_label: Label
var ai_performance_label: Label
var update_performance_label: Label
var speed_control: OptionButton
var thread_count_label: Label
var thread_slider: Slider
var status_label: Label
var save_ai_button: Button
var ball_speed_label: Label
var ball_speed_slider: Slider
var paddle_speed_label: Label
var paddle_speed_slider: Slider

var status_visible_timer: float = 0

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
    alive_ais_label = get_node(alive_ais_label_path)
    ai_performance_label = get_node(ai_performance_label_path)
    update_performance_label = get_node(update_performance_label_path)
    speed_control = get_node(speed_control_path)
    thread_count_label = get_node(thread_count_label_path)
    thread_slider = get_node(thread_slider_path)
    status_label = get_node(status_label_path)
    save_ai_button = get_node(save_ai_button_path)
    ball_speed_label = get_node(ball_speed_label_path)
    ball_speed_slider = get_node(ball_speed_slider_path)
    paddle_speed_label = get_node(paddle_speed_label_path)
    paddle_speed_slider = get_node(paddle_speed_slider_path)


# Called from C++ when match startup code has ran
func on_start():
    apply_visibility()

    var processors = OS.get_processor_count()
    thread_slider.max_value = processors

    # Set default thread count
    thread_slider.value = int(max(1, processors / 2))

    emit_signal("paddle_speed_changed", int(paddle_speed_slider.value))
    emit_signal("ball_speed_changed", int(ball_speed_slider.value))

    paddle_speed_label.text = "%s" % int(paddle_speed_slider.value)
    ball_speed_label.text = "%s" % int(ball_speed_slider.value)


func _process(delta):
    # We keep track of total run time (if we ever move to having the AI be
    # paused at the start, this needs changes)
    total_elapsed += delta

    fps_label.text = "FPS: %s" % Engine.get_frames_per_second()

    total_time_label.text = "Total Time: %s" % int(total_elapsed)
    game_time_label.text = "Game Time: %s" % stepify(elapsed_match_time, 0.1)
    score_label.text = "Score: %s" % stepify(score, 0.1)
    lives_label.text = "Lives: %s" % lives
    update_performance_label.text = "Game Update time: %sms" % stepify(update_performance, 0.1)

    if is_player:
        player_label.text = "Player: user"
    else:
        player_label.text = "Player: AI"
        generation_label.text = "Generation: %s" % generation
        ai_id_label.text = "Watching AI: %s" % ai_id
        alive_ais_label.text = "Alive AIs: %s" % alive_ais
        ai_performance_label.text = "AI + match sim: %sms" % stepify(ai_performance, 0.1)

    save_ai_button.disabled = generation < 2

    if status_visible_timer > 0:
        status_visible_timer -= delta

        if status_visible_timer <= 0:
            status_label.text = ""
            # Should this be hidden?
            # status_label.visible = false

func apply_visibility():
    # ai_start_button.visible = !is_player
    # For now the AI autostarts so this button is never used
    ai_start_button.visible = false
    ai_controls_container.visible = !is_player
    ai_stats_container.visible = !is_player

func show_status(status: String):
    status_label.visible = true
    status_visible_timer = status_hide_time
    status_label.text = status

func _on_QuitButton_pressed():
    if get_tree().change_scene("res://src/MainMenu.tscn") != OK:
        printerr("failed to load main menu scene")



func _on_Training_Speed_item_selected(index):
    var new_speed: int = speed_control.get_item_id(index)

    emit_signal("training_speed_changed", new_speed)


func _on_Threads_value_changed(value):
    value = int(value)
    emit_signal("threads_changed", value)
    on_threads_updated(value)

func on_threads_updated(value):
    thread_count_label.text = "%s" % value


func _on_SaveAI_pressed():
    emit_signal("save_ai_pressed")


func _on_BallSpeed_value_changed(value):
    emit_signal("ball_speed_changed", int(value))
    ball_speed_label.text = "%s" % int(value)


func _on_PaddleSpeed_value_changed(value):
    emit_signal("paddle_speed_changed", int(value))
    paddle_speed_label.text = "%s" % int(value)
