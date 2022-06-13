extends KinematicBody2D

var speed = 200

func _physics_process(delta) -> void:
	var velocity = get_velocity()
	move_and_slide(velocity)
	
func get_velocity() -> Vector2:
	var v = Vector2()
	if Input.is_action_pressed("ui_up"):
		v.y -= 1
	if Input.is_action_pressed("ui_down"):
		v.y += 1
	if Input.is_action_pressed("ui_left"):
		v.x -= 1
	if Input.is_action_pressed("ui_right"):
		v.x += 1
	return v.normalized() * speed
	
