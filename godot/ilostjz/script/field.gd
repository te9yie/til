extends Node2D

const Block = preload("res://script/block.gd")

var count: int = 0;
var index: int = 0;

func _process(delta: float) -> void:
	count += 1
	if count % 30 == 0:
		index += 1
		update()

func _draw() -> void:
	draw_blocks(0, 0, Block.BlockType.I)
	draw_blocks(100, 0, Block.BlockType.L)
	draw_blocks(200, 0, Block.BlockType.O)
	draw_blocks(300, 0, Block.BlockType.S)
	draw_blocks(0, 100, Block.BlockType.T)
	draw_blocks(100, 100, Block.BlockType.J)
	draw_blocks(200, 100, Block.BlockType.Z)

func draw_blocks(x: int, y: int, type: int) -> void:
	var shape = Block.get_shape(type, index)
	if shape.size() > 0:
		for iy in range(4):
			for ix in range(4):
				if shape[iy * 4 + ix] == 1:
					draw_rect(Rect2(x + ix * 20, y + iy * 20, 20, 20), Block.get_color(type))
