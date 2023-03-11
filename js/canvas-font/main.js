class Label {
  constructor(ctx, text) {
    this.text = text;
    this.metrics = ctx.measureText(text);
  }
  fill(ctx, x, y) {
    ctx.fillText(this.text, x, y + this.metrics.fontBoundingBoxAscent);
  }
  width() {
    return this.metrics.width;
  }
  height() {
    return (
      this.metrics.fontBoundingBoxAscent + this.metrics.fontBoundingBoxDescent
    );
  }
}

const createCanvas = () => {
  const canvas = document.createElement("canvas");
  canvas.width = 16 * 40;
  canvas.height = 9 * 40;
  canvas.style.border = "1px solid";
  return canvas;
};

const drawTextInfo = (ctx, text, x, y) => {
  const metrics = ctx.measureText(text);
  const height = metrics.fontBoundingBoxAscent + metrics.fontBoundingBoxDescent;
  const members = [
    "actualBoundingBoxAscent",
    "actualBoundingBoxDescent",
    "actualBoundingBoxLeft",
    "actualBoundingBoxRight",
    "fontBoundingBoxAscent",
    "fontBoundingBoxDescent",
  ];
  ctx.fillText(text, x, y + metrics.fontBoundingBoxAscent);
  for (let i = 0; i < members.length; ++i) {
    ctx.fillText(
      `${members[i]}: ${metrics[members[i]]}`,
      x,
      y + metrics.fontBoundingBoxAscent + height * (i + 1)
    );
  }
};

document.addEventListener("DOMContentLoaded", () => {
  const canvas = createCanvas();
  document.body.appendChild(canvas);

  const ctx = canvas.getContext("2d");
  ctx.font = "12px sans-serif";

  const hello = new Label(ctx, "Hello");
  hello.fill(ctx, 10, 0);

  drawTextInfo(
    ctx,
    "the quick brown fox jumps over the lazy dog",
    10,
    hello.height()
  );
});
