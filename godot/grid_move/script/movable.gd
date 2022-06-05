extends Node2D

func move(x: int, y: int, duration: float):
	var tween: Tween = get_node("Tween")
	tween.interpolate_property(self, "position", position, position + Vector2(x, y), duration)
	tween.start()

func is_moving():
	var tween: Tween = get_node("Tween")
	return tween.is_active()
