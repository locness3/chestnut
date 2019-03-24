#version 330

uniform float red_min;
uniform float red_mid;
uniform float red_max;

uniform float green_min;
uniform float green_mid;
uniform float green_max;

uniform float blue_min;
uniform float blue_mid;
uniform float blue_max;

uniform sampler2D myTexture;

in vec2 vTexCoord;
out vec4 color;

float adjust(float val, float min_val, float mid_val, float max_val) 
{
	if (val <= min_val/255) {
	  val = 0.0;
	} else if (val >= max_val/255) {
	  val = 1.0;
  } else { 
    val = val - (min_val/255) + ((255-max_val)/255);
    // linearly scale and apply mid/gamma. currently gamma
    val = val * mid_val/128;    
  }
  return val;
}

void main(void) 
{
	vec4 textureColor = texture2D(myTexture, vTexCoord);

	vec3 rgb = textureColor.rgb;
	
  rgb.r = adjust(rgb.r, red_min, red_mid, red_max);
  rgb.g = adjust(rgb.g, green_min, green_mid, green_max);
  rgb.b = adjust(rgb.b, blue_min, blue_mid, blue_max);

	color = vec4(rgb.r, rgb.g, rgb.b, textureColor.a);
}
