#version 330

uniform int mode;

uniform sampler2D myTexture;

in vec2 vTexCoord;
out vec4 color;

float maxVal(vec3 pixel)
{
  float val = 0;
  if (pixel.r > val) {
    val = pixel.r;
  }
  if (pixel.g > val) {
    val = pixel.g;
  }
  if (pixel.b > val) {
    val = pixel.b;
  }
  return val;
}

float averageVal(vec3 pixel)
{
  return (pixel.r + pixel.g + pixel.b) / 3;
}

float luminosityVal(vec3 pixel)
{
  return (0.2126 * pixel.r) + (0.7152 * pixel.g) + (0.0722 * pixel.b);
}

float lightnessVal(vec3 pixel)
{
  float maximum = max(pixel.r, pixel.g);
  maximum = max(maximum, pixel.b);
  float minimum = min(pixel.r, pixel.g);
  minimum = min(minimum, pixel.b);

  return (maximum + minimum) / 2;
}

void main(void) 
{
  vec4 textureColor = texture(myTexture, vTexCoord);

  vec3 rgb = textureColor.rgb;

  float val = 0;

  switch (mode) {
  case 0:
    val = maxVal(rgb);
    break;
  case 1:
    val = averageVal(rgb);
    break;
  case 2:
    val = luminosityVal(rgb);
    break;
  case 3:
    val = lightnessVal(rgb);
  };

  color = vec4(val, val, val, textureColor.a);
}
