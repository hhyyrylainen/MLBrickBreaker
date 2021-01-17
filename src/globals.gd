extends Node

# Autoload that holds some info that needs to be preserved over scene changes

var is_player: bool


# Called when the node enters the scene tree for the first time.
func _ready():
    is_player = false


# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#    pass
