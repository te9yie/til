extends Node2D

const font_data: DynamicFontData = preload("res://font/MPLUS2-Bold.ttf")

var font: DynamicFont = DynamicFont.new()

func _ready() -> void:
	font.font_data = font_data
	font.size = 26

func _draw() -> void:
	draw_string(font, Vector2(0, 0), "I LOST JZ", Color.white)
