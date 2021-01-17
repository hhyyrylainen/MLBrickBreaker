extends Control


# Declare member variables here. Examples:
# var a = 2
# var b = "text"

var wants_to_begin_game = false


onready var game_scene = load("res://src/Game.tscn")

# Called when the node enters the scene tree for the first time.
func _ready():
    pass


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
    if wants_to_begin_game:
        wants_to_begin_game = false
        if get_tree().change_scene_to(game_scene) != OK:
            printerr("can't load game scene")


func _on_QuitButton_pressed():
    get_tree().quit()


func _on_Button_pressed():
    Globals.is_player = true
    wants_to_begin_game = true


func _on_TrainAI_pressed():
    Globals.is_player = false
    wants_to_begin_game = true

