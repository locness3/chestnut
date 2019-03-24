#version 330

uniform bool avg;
uniform bool maxi;
uniform bool luma;
uniform bool light;

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
    return (0.21 * pixel.r) + (0.72 * pixel.g) + (0.07 * pixel.b);
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

    if (avg) {
        val = averageVal(rgb);
    } else if (maxi) {
        val = maxVal(rgb);
    } else if (luma) {
        val = luminosityVal(rgb);
    } else if (light) {
        val = lightnessVal(rgb);
    }


    color = vec4(val, val, val, textureColor.a);
}
