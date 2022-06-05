extends Node

const Movable = preload("res://script/movable.gd")
const GRID_SIZE = 8

onready var player: Movable = $"YSort/Movable"

func _process(delta):
	if not player or player.is_moving():
		return
	var velocity = Vector2.ZERO
	if Input.is_action_pressed("ui_up"):
		velocity.y -= 1
	if Input.is_action_pressed("ui_down"):
		velocity.y += 1
	if Input.is_action_pressed("ui_left"):
		velocity.x -= 1
	if Input.is_action_pressed("ui_right"):
		velocity.x += 1
	if velocity.length() > 0:
		velocity *= GRID_SIZE
		player.move(velocity.x, velocity.y, 0.2)
