extends Node2D

const Block = preload("res://script/block.gd")

const WIDTH: int = 10
const HEIGHT: int = 21
const DRAW_OFFSET: int = 1

var blocks: Array
var control_x: int = 0
var control_y: int = 0
var control_type: int = 0
var control_rotation: int = 0

func reset_field() -> void:
	blocks.resize(WIDTH * HEIGHT)
	for i in range(blocks.size()):
		blocks[i] = Block.BlockType.NONE
	for i in range(HEIGHT):
		blocks[i * WIDTH] = i % Block.BlockType.COUNT

func reset_control(type: int) -> void:
	control_x = 3
	control_y = 0
	control_type = type
	control_rotation = 0

func _ready() -> void:
	reset_field()
	reset_control(Block.BlockType.L)

func draw_field() -> void:
	for dy in range(HEIGHT - DRAW_OFFSET):
		for dx in range(WIDTH):
			var index = (dy + DRAW_OFFSET) * WIDTH + dx
			if blocks[index] != Block.BlockType.NONE:
				Block.draw_block(self, dx, dy, blocks[index])

func draw_control() -> void:
	if control_type == Block.BlockType.NONE: return
	var shape = Block.get_shape(control_type, control_rotation)
	for dy in range(4):
		for dx in range(4):
			var y = control_y + dy - DRAW_OFFSET
			if y < 0 or HEIGHT - DRAW_OFFSET <= y: continue
			var index = dy * 4 + dx
			if shape[index] != Block.BlockType.NONE:
				Block.draw_block(self, control_x + dx, y, shape[index])

var count: int = 0

func _process(delta: float) -> void:
	count += 1
	if count % 30 == 0:
		control_rotation += 1
		control_y += 1
		if control_y > HEIGHT:
			control_y = 0
		update()

func _draw() -> void:
	draw_field()
	draw_control()
