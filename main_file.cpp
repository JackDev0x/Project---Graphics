/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "model.cpp"

#include <Importer.hpp>
#include <scene.h>
#include <postprocess.h>

float robot_speed = 0; //prędkość chodzenia robota
float max_robot_speed = 0.05f; //maksymalna prędkość chodzenia robota
float robot_rotation_speed = 0;

float cam_speed_x = 0;
float cam_speed_y = 0;
float max_cam_speed = 1; //maksymalna prędkość kamery

double auto_time = 0;
double t1 = 0.5f;
double t2 = 0.6f;
int sign = 1;

bool is_pressed_w = false;
bool is_pressed_s = false;
bool is_pressed_a = false;
bool is_pressed_d = false;

//bool auto_pilot = true;

float aspectRatio=1;

glm::vec3 cam_position = glm::vec3(0, -25, 20);
glm::vec3 cam_looking_p = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 pos = glm::vec3();

ShaderProgram *sp;

Model robot; //Model robota
Model terrain; //Model terenu
Model tree1; //Model drzewa
Model tree2; 
Model lamp; //Model lampy

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}


void keyCallback(GLFWwindow* window,int key,int scancode,int action,int mods) {
	if (action == GLFW_PRESS) {
		//kamera
		if (key == GLFW_KEY_LEFT) cam_speed_x = -max_cam_speed;
		if (key == GLFW_KEY_RIGHT) cam_speed_x = max_cam_speed;
		if (key == GLFW_KEY_UP) cam_speed_y = max_cam_speed;
		if (key == GLFW_KEY_DOWN) cam_speed_y = -max_cam_speed;
		//robot
		if (key == GLFW_KEY_A) {
			robot_rotation_speed = PI / 8;
			is_pressed_a = true;
			if (!is_pressed_w && !is_pressed_s)
				robot_speed = 0;
		}
		if (key == GLFW_KEY_D) {
			robot_rotation_speed = -PI / 8;
			is_pressed_d = true;
			if (!is_pressed_w && !is_pressed_s)
				robot_speed = 0;
		}
		if (key == GLFW_KEY_W) { robot_speed = max_robot_speed; is_pressed_w = true; }
		if (key == GLFW_KEY_S) { robot_speed = -max_robot_speed/2; is_pressed_s = true; }
    }
    if (action==GLFW_RELEASE) {
		//kamera
        if (key==GLFW_KEY_LEFT) cam_speed_x = 0;
        if (key==GLFW_KEY_RIGHT) cam_speed_x = 0;
		if (key == GLFW_KEY_UP) cam_speed_y = 0;
		if (key == GLFW_KEY_DOWN) cam_speed_y = 0;
		//robot
		if (key == GLFW_KEY_A) { robot_rotation_speed = 0; is_pressed_a = false; }
		if(key == GLFW_KEY_D) { robot_rotation_speed = 0; is_pressed_d = false; }
		if (key == GLFW_KEY_W) { robot_speed = 0; is_pressed_w = false; }
		if (key == GLFW_KEY_S) { robot_speed = 0; is_pressed_s = false; }

    }
}

void windowResizeCallback(GLFWwindow* window,int width,int height) {
    if (height==0) return;
    aspectRatio=(float)width/(float)height;
    glViewport(0,0,width,height);
}


GLuint readTexture(const char* filename) {
	GLuint tex;
    glActiveTexture(GL_TEXTURE0);
	//if (n == 1)glActiveTexture(GL_TEXTURE1);

    //Wczytanie do pamięci komputera
    std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
    unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
    //Wczytaj obrazek
    unsigned error = lodepng::decode(image, width, height, filename);

    //Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
    //Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
    glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return tex;
}

void loadModel(std::string plik, Model* model) {
	using namespace std;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
	cout << importer.GetErrorString() << endl;

	aiMesh* mesh = scene->mMeshes[0];
	for (int i = 0; i < mesh->mNumVertices; i++) {
		aiVector3D vertex = mesh->mVertices[i];
		//cout << vertex.x << " " << vertex.y << vertex.z << endl;
		model->verts.push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1));

		aiVector3D normal = mesh->mNormals[i];
		//cout << normal.x << " " << normal.y << normal.z << endl;
		model->norms.push_back(glm::vec4(normal.x, normal.y, normal.z, 0));

		//unsigned int liczba_zest = mesh->GetNumUVChannels();
		//unsigned int wymiar_wsp_tex = mesh->mNumUVComponents[0];

		aiVector3D texCoord = mesh->mTextureCoords[0][i];
		//cout << texCoord.x << " " << texCoord.y << endl;
		model->texCoords.push_back(glm::vec2(texCoord.x,texCoord.y));
	}

	for (int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (int j = 0; j < face.mNumIndices; j++) {
			model->indices.push_back(face.mIndices[j]);
		}
	}

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	/*for (int i = 0; i < 19; i++) {
		cout << i << " " << material->GetTextureCount((aiTextureType)i) << endl;
	}*/

	for (int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++) {
		aiString str;
		/*aiTextureMapping mapping;
		unsigned int uvMapping;
		ai_real blend;
		aiTextureOp op;
		aiTextureMapMode mapMode;
		material->GetTexture(aiTextureType_DIFFUSE, i, &str, &mapping, &uvMapping, &blend, &op, &mapMode);*/
		material->GetTexture(aiTextureType_DIFFUSE, i, &str);
		cout << str.C_Str() << endl;
	}
}

//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0,0,0,1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window,windowResizeCallback);
	glfwSetKeyCallback(window,keyCallback);

	sp=new ShaderProgram("v_simplest.glsl",NULL,"f_simplest.glsl");
	robot.tex = readTexture("robot_diff.png");
	terrain.tex = readTexture("terrain_diff.png");
	tree1.tex = readTexture("tree_diff.png");
	tree2.tex = readTexture("tree_diff.png");
	lamp.tex = readTexture("lamp_diff.png");
	loadModel(std::string("terrain.obj"), &terrain);
	loadModel(std::string("robot.obj"), &robot);
	loadModel(std::string("tree.obj"), &tree1);
	loadModel(std::string("tree.obj"), &tree2);
	loadModel(std::string("lamp.obj"), &lamp);
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
	glDeleteTextures(1, &terrain.tex);
	glDeleteTextures(1, &robot.tex);
    delete sp;
}

void drawObject(Model object, glm::vec3 position) {
	glm::mat4 M_object = glm::mat4(1.0f);
	M_object = glm::translate(M_object, position);
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_object));

	glEnableVertexAttribArray(sp->a("vertex"));
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, object.verts.data());

	glEnableVertexAttribArray(sp->a("normal"));
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, object.norms.data());

	glEnableVertexAttribArray(sp->a("texCoord"));
	glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, object.texCoords.data());

	//glUniform1i(sp->u("tex"), 1);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, object.tex);

	glDrawElements(GL_TRIANGLES, object.indices.size(), GL_UNSIGNED_INT, object.indices.data());
}


//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window,float angle_z, float speed) {
	//************Tutaj umieszczaj kod rysujący obraz******************
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Wylicz macierz widoku
	glm::mat4 V=glm::lookAt(
		 cam_position,
         cam_looking_p,
         glm::vec3(0.0f,0.0f,1.0f)); 

	//Wylicz macierz rzutowania
    glm::mat4 P=glm::perspective(50.0f*PI/180.0f, aspectRatio, 0.01f, 100.0f); 

	//wyliczanie pozycji kamery
	cam_position += glm::vec3(cam_speed_x, cam_speed_y, 0.0f);
	cam_looking_p += glm::vec3(cam_speed_x, cam_speed_y,0.0f);
	
	glm::mat4 M = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		pos.x, pos.y, 0.0f, 1.0f
	);

	M = glm::rotate(M, angle_z, glm::vec3(0.0f, 0.0f, 1.0f));
	M = glm::translate(M, glm::vec3(0.0f, speed, 0.0f));

	pos = glm::vec3(M[3][0], M[3][1], 0.0f);
	//std::cout << M[3][0] << " " << M[3][1] << " " << M[3][2] << std::endl;
	
    sp->use();//Aktywacja programu cieniującego
    //Przeslij parametry programu cieniującego do karty graficznej
    glUniformMatrix4fv(sp->u("P"),1,false,glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"),1,false,glm::value_ptr(V));
    glUniformMatrix4fv(sp->u("M"),1,false,glm::value_ptr(M));

    glEnableVertexAttribArray(sp->a("vertex"));  //Włącz przesyłanie danych do atrybutu vertex
	glVertexAttribPointer(sp->a("vertex"),4,GL_FLOAT,false,0, robot.verts.data()); //Wskaż tablicę z danymi dla atrybutu vertex

	//glEnableVertexAttribArray(sp->a("color"));  //Włącz przesyłanie danych do atrybutu color
	//glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, colors); //Wskaż tablicę z danymi dla atrybutu color

	glEnableVertexAttribArray(sp->a("normal"));  //Włącz przesyłanie danych do atrybutu normal
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, robot.norms.data()); //Wskaż tablicę z danymi dla atrybutu normal

	glEnableVertexAttribArray(sp->a("texCoord"));  //Włącz przesyłanie danych do atrybutu texCoord
	glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, robot.texCoords.data()); //Wskaż tablicę z danymi dla atrybutu texCoord

	glUniform1i(sp->u("tex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, robot.tex);
	
	glDrawElements(GL_TRIANGLES, robot.indices.size(), GL_UNSIGNED_INT, robot.indices.data());

	//-----Teren------
	drawObject(terrain, glm::vec3());

	//-----Drzewo------
	drawObject(tree1, glm::vec3(20.0f, 20.0f, 0.0f));
	drawObject(tree2, glm::vec3(-10.0f, 15.0f, 0.0f));
	drawObject(lamp, glm::vec3(15.0f, 15.0f, 0.0f));


	glDisableVertexAttribArray(sp->a("vertex"));  //Wyłącz przesyłanie danych do atrybutu vertex
	glDisableVertexAttribArray(sp->a("normal"));  //Wyłącz przesyłanie danych do atrybutu normal
	glDisableVertexAttribArray(sp->a("texCoord"));  //Wyłącz przesyłanie danych do atrybutu texCoord0

	glm::vec4 r1_pos = glm::vec4(15.0f, 15.0f, 6.4f, 1.0f);
	//::cout << r1_pos.x << " " << r1_pos.y << " " << r1_pos.z << " " << std::endl;
	glUniform4fv(sp->u("rf1"), 1, glm::value_ptr(r1_pos));

    glfwSwapBuffers(window); //Przerzuć tylny bufor na przedni

}

void autoPilot() {
	robot_speed = max_robot_speed;
	auto_time += glfwGetTime();

	if (auto_time > t1)
		robot_rotation_speed = sign * PI / 8;
	if (auto_time > t2) {
		robot_rotation_speed = 0;
		auto_time = 0;
		srand(time(NULL));
		t1 = (rand() % 40 + 5) / 10.0f;
		t2 = t1 + (rand() % 40 + 5) / 10.0f;
		sign = rand() % 3 - 1;
		std::cout << sign << std::endl;
	}
}


int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(1000, 1000, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące

	//Główna pętla
	float angle_x=0; //Aktualny kąt obrotu obiektu
	float angle_z=0; //Aktualny kąt obrotu obiektu
	float walk_speed = 0;
	glfwSetTime(0); //Zeruj timer
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		if (!is_pressed_a && !is_pressed_d && !is_pressed_w && !is_pressed_s) {
			autoPilot();
		}

        angle_z+= robot_rotation_speed*glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		//walk_speed += robot_speed *glfwGetTime();
		
        glfwSetTime(0); //Zeruj timer
		drawScene(window,angle_z,robot_speed); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
