class Context {
  constructor(canvas) {
    this.canvas = canvas;
    this.ctx = canvas.getContext("2d");
    this.dragItem = null;
    this.dragPos = { x: 0, y: 0 };
  }

  getCanvasPos(e) {
    const rect = this.canvas.getBoundingClientRect();
    return {
      x: e.clientX - rect.left,
      y: e.clientY - rect.top,
    };
  }
}

const onmousedown = (ctx, e) => {
  ctx.dragItem = 1;
  ctx.dragPos = ctx.getCanvasPos(e);
  ondraw(ctx);
};
const onmouseup = (ctx, e) => {
  ctx.dragItem = null;
};
const onmousemove = (ctx, e) => {};

const ondraw = (ctx) => {
  ctx.ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
  if (ctx.box && ctx.dragItem) {
    ctx.box.draw(ctx.ctx, ctx.dragPos.x, ctx.dragPos.y);
  }
};

class Label {
  constructor(ctx, text) {
    this.text = text;
    this.metrics = ctx.measureText(text);
  }
  fill(ctx, x, y) {
    ctx.fillText(this.text, x, y + this.metrics.fontBoundingBoxAscent);
  }
  size() {
    return {
      w: this.metrics.width,
      h:
        this.metrics.fontBoundingBoxAscent +
        this.metrics.fontBoundingBoxDescent,
    };
  }
}

class Box {
  constructor(inner) {
    this.inner = inner;
    this.margin = 10;
  }
  draw(ctx, x, y) {
    const size = this.size();
    ctx.strokeRect(x, y, size.w, size.h);
    this.inner.fill(ctx, x + this.margin, y + this.margin);
  }
  size() {
    const innerSize = this.inner.size();
    return {
      w: innerSize.w + this.margin * 2,
      h: innerSize.h + this.margin * 2,
    };
  }
}

const createCanvas = () => {
  const canvas = document.createElement("canvas");
  canvas.width = 16 * 40;
  canvas.height = 9 * 40;
  canvas.style.border = "1px solid";
  return canvas;
};

document.addEventListener("DOMContentLoaded", () => {
  const canvas = createCanvas();
  document.body.appendChild(canvas);

  const context = new Context(canvas);

  const ctx = canvas.getContext("2d");
  ctx.font = "12px sans-serif";
  ctx.strokeStyle = "black";

  const box = new Box(new Label(ctx, "Hello"));
  context.box = box;

  canvas.addEventListener("mousedown", (e) => {
    onmousedown(context, e);
  });
  canvas.addEventListener("mouseup", (e) => {
    onmouseup(context, e);
  });
  canvas.addEventListener("mousemove", (e) => {
    onmousemove(context, e);
  });
});
