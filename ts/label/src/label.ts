import { Pos, Size } from "./geometry";

export class Label {
  constructor(private text: string) {}

  update_layout(ctx: CanvasRenderingContext2D, hintSize: Size): Size {
    const m = ctx.measureText(this.text);
    return new Size(
      m.width,
      m.fontBoundingBoxAscent + m.fontBoundingBoxDescent
    );
  }

  render(ctx: CanvasRenderingContext2D, p: Pos) {
    const m = ctx.measureText(this.text);
    ctx.fillText(this.text, p.x, p.y + m.fontBoundingBoxAscent);
  }
}
