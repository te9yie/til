import { createCanvas } from "https://deno.land/x/canvas/mod.ts";

const canvas = createCanvas(200, 200);
const ctx = canvas.getContext("2d");

ctx.fillStyle = "red";
ctx.fillRect(50, 50, 200 - 100, 200 - 100);

await Deno.writeFile("image.png", canvas.toBuffer());
