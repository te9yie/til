const WIDTH = 400;
const HEIGHT = 400;

// const setup = () => {};
// thisを持たないのでアロー関数はダメ。
function setup() {
  createCanvas(WIDTH, HEIGHT);
}

function draw() {
  background(220);
  textSize(20);
  textAlign(CENTER, CENTER);
  text("Hello🧓", WIDTH / 2, HEIGHT / 2);
}
