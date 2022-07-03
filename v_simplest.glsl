#version 330

//Zmienne jednorodne
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec4 rf1;

//Atrybuty
in vec4 vertex; //wspolrzedne wierzcholka w przestrzeni modelu
in vec4 color; //kolor związany z wierzchołkiem
in vec4 normal; //wektor normalny w przestrzeni modelu
in vec2 texCoord;

//Zmienne interpolowane
out vec4 ic;
out vec4 l;
out vec4 l_rf;
out vec4 n;
out vec4 v;
out vec2 iTexCoord0;
out vec2 iTexCoord1;

void main(void) {
    vec4 lp = vec4(-20, 0, 50, 1); //współrzędne światła

    //wektor do światła w przestrzeni oka
    l = normalize(V * lp - V*M*vertex);
    l_rf = normalize(V * rf1 - V*M*vertex);

    v = normalize(vec4(0, 0, 0, 1) - V * M * vertex); //wektor do obserwatora w przestrzeni oka
    n = normalize(V * M * normal); //wektor normalny w przestrzeni oka
    iTexCoord0 = texCoord;
    iTexCoord1 = (n.xy + 1) / 2;
    ic = color;
    
    gl_Position=P*V*M*vertex;
}
