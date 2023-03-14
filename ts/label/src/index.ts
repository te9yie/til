import { Pos, Size } from "./geometry";
import { Label } from "./label";

document.addEventListener("DOMContentLoaded", () => {
  const canvas = document.createElement("canvas") as HTMLCanvasElement;
  const ctx = canvas.getContext("2d") as CanvasRenderingContext2D;
  canvas.style.border = "1px solid";
  ctx.font = "12px sans-serif";

  const label = new Label("the quick brown fox jumps over the lazy dog");
  const s = label.update_layout(ctx, new Size(canvas.width, canvas.height));
  const offset = new Pos(10, 10);
  label.render(ctx, offset);
  label.render(ctx, new Pos(offset.x, offset.y + s.h));

  ctx.strokeRect(offset.x, offset.y, s.w, s.h * 2);

  document.body.appendChild(canvas);
});
