enum BlockType {
	NONE,
	I,
	L,
	O,
	S,
	T,
	J,
	Z,
	COUNT
}

const BLOCK_SIZE = 20

const I_SHAPE: Array = [
	[
		0, 0, 0, 0,
		1, 1, 1, 1,
		0, 0, 0, 0,
		0, 0, 0, 0,
	],
	[
		0, 0, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0,
	],
]
const L_SHAPE: Array = [
	[
		0, 0, 0, 0,
		1, 1, 1, 0,
		1, 0, 0, 0,
		0, 0, 0, 0,
	],
	[
		1, 1, 0, 0,
		0, 1, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 0,
	],
	[
		0, 0, 0, 0,
		0, 0, 1, 0,
		1, 1, 1, 0,
		0, 0, 0, 0,
	],
	[
		0, 1, 0, 0,
		0, 1, 0, 0,
		0, 1, 1, 0,
		0, 0, 0, 0,
	],
]
const O_SHAPE: Array = [
	[
		0, 0, 0, 0,
		0, 1, 1, 0,
		0, 1, 1, 0,
		0, 0, 0, 0,
	],
]
const S_SHAPE: Array = [
	[
		0, 0, 0, 0,
		0, 1, 1, 0,
		1, 1, 0, 0,
		0, 0, 0, 0,
	],
	[
		1, 0, 0, 0,
		1, 1, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 0,
	],
]
const T_SHAPE: Array = [
	[
		0, 0, 0, 0,
		1, 1, 1, 0,
		0, 1, 0, 0,
		0, 0, 0, 0,
	],
	[
		0, 1, 0, 0,
		1, 1, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 0,
	],
	[
		0, 0, 0, 0,
		0, 1, 0, 0,
		1, 1, 1, 0,
		0, 0, 0, 0,
	],
	[
		0, 1, 0, 0,
		0, 1, 1, 0,
		0, 1, 0, 0,
		0, 0, 0, 0,
	],
]
const J_SHAPE: Array = [
	[
		0, 0, 0, 0,
		1, 1, 1, 0,
		0, 0, 1, 0,
		0, 0, 0, 0,
	],
	[
		0, 1, 0, 0,
		0, 1, 0, 0,
		1, 1, 0, 0,
		0, 0, 0, 0,
	],
	[
		0, 0, 0, 0,
		1, 0, 0, 0,
		1, 1, 1, 0,
		0, 0, 0, 0,
	],
	[
		0, 1, 1, 0,
		0, 1, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 0,
	],
]
const Z_SHAPE: Array = [
	[
		0, 0, 0, 0,
		1, 1, 0, 0,
		0, 1, 1, 0,
		0, 0, 0, 0,
	],
	[
		0, 0, 1, 0,
		0, 1, 1, 0,
		0, 1, 0, 0,
		0, 0, 0, 0,
	],
]

static func get_color(type: int) -> Color:
	match type:
		BlockType.I: return Color.red
		BlockType.L: return Color.orange
		BlockType.O: return Color.yellow
		BlockType.S: return Color.purple
		BlockType.T: return Color.cyan
		BlockType.J: return Color.blue
		BlockType.Z: return Color.green
		_: return Color.white

static func get_shape(type: int, index: int) -> Array:
	match type:
		BlockType.I:
			return I_SHAPE[index % I_SHAPE.size()]
		BlockType.L:
			return L_SHAPE[index % L_SHAPE.size()]
		BlockType.O:
			return O_SHAPE[index % O_SHAPE.size()]
		BlockType.S:
			return S_SHAPE[index % S_SHAPE.size()]
		BlockType.T:
			return T_SHAPE[index % T_SHAPE.size()]
		BlockType.J:
			return J_SHAPE[index % J_SHAPE.size()]
		BlockType.Z:
			return Z_SHAPE[index % Z_SHAPE.size()]
		_:
			return []

static func draw_block(g: Node2D, x: int, y: int, type: int) -> void:
	g.draw_rect(Rect2(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE), get_color(type))
