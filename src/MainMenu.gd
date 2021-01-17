extends Control


# Declare member variables here. Examples:
# var a = 2
# var b = "text"

onready var game_scene = load("res://src/Game.tscn")

# Called when the node enters the scene tree for the first time.
func _ready():
    pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#    pass


func _on_QuitButton_pressed():
    get_tree().quit()


func _on_Button_pressed():
    var tree = get_parent()

    self.queue_free()

    tree.remove_child(self)

    var game = game_scene.instance()

    game.PlayerControlled = true

    tree.add_child(game)


func _on_TrainAI_pressed():
    if get_tree().change_scene_to(game_scene) != OK:
        printerr("can't load game scene")
