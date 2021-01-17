extends Control


# Declare member variables here. Examples:
# var a = 2
# var b = "text"

export var fps_label_path: NodePath
export var player_label_path: NodePath

var is_player: bool = false

var fps_label: Label
var player_label: Label


# Called when the node enters the scene tree for the first time.
func _ready():
    fps_label = get_node(fps_label_path)
    player_label = get_node(player_label_path)

    is_player = Globals.is_player


func _process(delta):
    fps_label.text = "FPS: %s" % Engine.get_frames_per_second()

    if is_player:
        player_label.text = "Player: user"
    else:
        player_label.text = "Player: AI"


func _on_QuitButton_pressed():
    if get_tree().change_scene("res://src/MainMenu.tscn") != OK:
        printerr("failed to load main menu scene")
