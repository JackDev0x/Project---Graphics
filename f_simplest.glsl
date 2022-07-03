#version 330

uniform sampler2D textureMap0;
uniform sampler2D textureMap1;

out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela

in vec4 ic; 
in vec4 n;
in vec4 l;
in vec4 l_rf;
in vec4 v;
in vec2 iTexCoord0;
in vec2 iTexCoord1;

void main(void) {

	//Znormalizowane interpolowane wektory
	vec4 ml = normalize(l);
	vec4 ml_rf = normalize(l_rf);
	vec4 mn = normalize(n);
	vec4 mv = normalize(v);

	//Wektor odbity
	vec4 mr = reflect(-ml, mn);
	vec4 mr_rf = reflect(-ml_rf, mn);

	//Parametry powierzchni
	vec4 kd = mix(texture(textureMap0,iTexCoord0),texture(textureMap1, iTexCoord1),0.3);
	vec4 ks = vec4(1,1,1,1);

	//Obliczenie modelu oświetlenia
	float nl = clamp(dot(mn, ml), 0, 1);
	float rv = pow(clamp(dot(mr, mv), 0, 1), 64);
	float nl_rf = clamp(dot(mn, ml_rf), 0, 1);
	//float rv_rf = pow(clamp(dot(mr_rf, mv), 0, 1), 1000);
	pixelColor = vec4(kd.rgb * (nl + nl_rf), kd.a) + vec4(ks.rgb*(rv), 0);
}
