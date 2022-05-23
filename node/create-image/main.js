const fs = require("fs");
const { createCanvas } = require("canvas");

const canvas = createCanvas(200, 200);
const ctx = canvas.getContext("2d");

ctx.fillStyle = "red";
ctx.fillRect(20, 20, 200 - 40, 200 - 40);

fs.writeFileSync("image.png", canvas.toBuffer("image/png"));
