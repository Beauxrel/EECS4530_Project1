#include <glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include "LoadShaders.h"
#include "linmath.h"
#include <map>
#include <vector>
using namespace std;
/*
 * Computer Graphics I -- Project 1 -- Base Code.
 * Name:
 *
 * This code is a framework/starting point for project 1 in the course.
 * you may feel free to use and modify this code for the project (in
 * fact I strongly suggest it!)
 *
 */

#define BUFFER_OFFSET(x)  ((const void*) (x))

GLuint programID;
/*
* Arrays to store the indices/names of the Vertex Array Objects and
* Buffers.  Rather than using the books enum approach I've just
* gone out and made a bunch of them and will use them as needed.
*
* This will change as we add things into our toolbox this term.
*/

GLuint vertexBuffers[10], arrayBuffers[10], elementBuffers[10];
/*
* Global variables
*   The location for the transformation and the current rotation
*   angle are set up as globals since multiple methods need to
*   access them.
*/
float rotationAngle;
int nbrTriangles[10];
mat4x4 rotation;
map<string, GLuint> locationMap;

// Prototypes
GLuint buildProgram(string vertexShaderName, string fragmentShaderName);
GLFWwindow* glfwStartUp(int& argCount, char* argValues[],
	string windowTitle = "No Title", int width = 500, int height = 500);
void setAttributes(float lineWidth = 1.0, GLenum face = GL_FRONT_AND_BACK,
	GLenum fill = GL_FILL);
void buildObjects();
void getLocations();
void init(string vertexShader, string fragmentShader);
float* readOBJFile(string filename, int& nbrTriangles, float*& normalArray);
/*
 * Error callback routine for glfw -- uses cstdio
 */
static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

/*
 * keypress callbacks for glfw -- Escape exits...
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static float currentAngle = 0.0;
	static float currentLimit = 1.0;
	mat4x4 viewingMatrix;
	mat4x4 projectionMatrix;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		currentAngle += 3.14159 / 18;  // 10 degrees
		vec3 eyePoint = { sin(currentAngle), 0.0f, cos(currentAngle) };
		vec3 upVector = { 0.0f, 1.0f, 0.0f };
		vec3 centerPoint = { 0.0f, 0.0f, 0.0f };
		mat4x4_look_at(viewingMatrix, eyePoint, centerPoint, upVector);
		GLuint viewingMatrixLocation = glGetUniformLocation(programID, "viewingMatrix");
		glUniformMatrix4fv(viewingMatrixLocation, 1, false, (const GLfloat*)viewingMatrix);
	}
	else if (key == GLFW_KEY_PERIOD && action == GLFW_PRESS) {
		currentAngle += 3.14159 / 18;  // 10 degrees
		vec3 eyePoint = { sin(currentAngle), 0.0f, cos(currentAngle) };
		vec3 upVector = { 0.0f, 1.0f, 0.0f };
		vec3 centerPoint = { 0.0f, 0.0f, 0.0f };
		mat4x4_look_at(viewingMatrix, eyePoint, centerPoint, upVector);
		GLuint viewingMatrixLocation = glGetUniformLocation(programID, "viewingMatrix");
		glUniformMatrix4fv(viewingMatrixLocation, 1, false, (const GLfloat*)viewingMatrix);
	}
	else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		currentAngle -= 3.14159 / 18;  // 10 degrees
		vec3 eyePoint = { sin(currentAngle), 0.0f, cos(currentAngle) };
		vec3 upVector = { 0.0f, 1.0f, 0.0f };
		vec3 centerPoint = { 0.0f, 0.0f, 0.0f };
		mat4x4_look_at(viewingMatrix, eyePoint, centerPoint, upVector);
		GLuint viewingMatrixLocation = glGetUniformLocation(programID, "viewingMatrix");
		glUniformMatrix4fv(viewingMatrixLocation, 1, false, (const GLfloat*)viewingMatrix);
	}
	else if (key == GLFW_KEY_COMMA && action == GLFW_PRESS) {
		currentAngle -= 3.14159 / 18;  // 10 degrees
		vec3 eyePoint = { sin(currentAngle), 0.0f, cos(currentAngle) };
		vec3 upVector = { 0.0f, 1.0f, 0.0f };
		vec3 centerPoint = { 0.0f, 0.0f, 0.0f };
		mat4x4_look_at(viewingMatrix, eyePoint, centerPoint, upVector);
		GLuint viewingMatrixLocation = glGetUniformLocation(programID, "viewingMatrix");
		glUniformMatrix4fv(viewingMatrixLocation, 1, false, (const GLfloat*)viewingMatrix);
	}
	else if (key == GLFW_KEY_MINUS && action == GLFW_PRESS) {
		currentLimit = currentLimit * 2;
		mat4x4_ortho(projectionMatrix, -currentLimit, currentLimit, -currentLimit, currentLimit, -100.0f, 100.0f);
		GLuint projectionMatrixLocation = glGetUniformLocation(programID, "projectionMatrix");
		glUniformMatrix4fv(projectionMatrixLocation, 1, false, (const GLfloat*)projectionMatrix);

	}
	else if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS) {
		currentLimit = currentLimit / 2;
		mat4x4_ortho(projectionMatrix, -currentLimit, currentLimit, -currentLimit, currentLimit, -100.0f, 100.0f);
		GLuint projectionMatrixLocation = glGetUniformLocation(programID, "projectionMatrix");
		glUniformMatrix4fv(projectionMatrixLocation, 1, false, (const GLfloat*)projectionMatrix);

	}

}

/*
 * Routine to encapsulate some of the startup routines for GLFW.  It returns the window ID of the
 * single window that is created.
 */
GLFWwindow* glfwStartUp(int& argCount, char* argValues[], string title, int width, int height) {
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);   // This is set to compliance for 4.1 -- if your system
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);   // supports 4.5 or 4.6 you may wish to modify it. 

	window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glfwSwapInterval(1);

	return window;
}


/*
 * Use the author's routines to build the program and return the program ID.
 */
GLuint buildProgram(string vertexShaderName, string fragmentShaderName) {

	/*
	*  Use the Books code to load in the shaders.
	*/
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, vertexShaderName.c_str() },
		{ GL_FRAGMENT_SHADER, fragmentShaderName.c_str() },
		{ GL_NONE, NULL }
	};
	GLuint program = LoadShaders(shaders);
	if (program == 0) {
		cerr << "GLSL Program didn't load.  Error \n" << endl
			<< "Vertex Shader = " << vertexShaderName << endl
			<< "Fragment Shader = " << fragmentShaderName << endl;
	}
	glUseProgram(program);
	return program;
}

/*
 * Set up the clear color, lineWidth, and the fill type for the display.
 */
void setAttributes(float lineWidth, GLenum face, GLenum fill) {
	/*
	* I'm using wide lines so that they are easier to see on the screen.
	* In addition, this version fills in the polygons rather than leaving it
	* as lines.
	*/
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glLineWidth(lineWidth);
	glPolygonMode(face, fill);
	glEnable(GL_DEPTH_TEST);

}

/*
 * read and/or build the objects to be displayed.  Also sets up attributes that are
 * vertex related.
 */

void buildObjects() {


	glGenVertexArrays(1, vertexBuffers);
	glBindVertexArray(vertexBuffers[0]);

	// Alternately...
	// GLuint   vaoID;
	// glGenVertexArrays(1, &vaoID);
	// glBindVertexArray(vaoID);
	//


	glGenBuffers(1, &(arrayBuffers[0]));
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	GLfloat* normals;
	GLfloat* vertices = readOBJFile("column.obj", nbrTriangles[0], normals);
	const int nbrVerticesPerTriangle = 3;
	const int nbrFloatsPerVertex = 4; // x, y, z, w
	const int nbrFloatsPerNormal = 3; // dx, dy, dz
	int  verticesSize = nbrTriangles[0] * nbrVerticesPerTriangle * nbrFloatsPerVertex * sizeof(GLfloat);
	int normalsSize = nbrTriangles[0] * nbrVerticesPerTriangle * nbrFloatsPerNormal * sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, verticesSize + normalsSize,
		NULL, GL_STATIC_DRAW);
	//                               offset in bytes   size in bytes     ptr to data    
	glBufferSubData(GL_ARRAY_BUFFER, 0, verticesSize, vertices);
	glBufferSubData(GL_ARRAY_BUFFER, verticesSize, normalsSize, normals);
	/*
	 * Set up variables into the shader programs (Note:  We need the
	 * shaders loaded and built into a program before we do this)
	 */
	GLuint vPosition = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(programID, "vNormal");
	if (vNormal != -1) {
		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(verticesSize));
	}

	glGenVertexArrays(1, &vertexBuffers[1]);
	glBindVertexArray(vertexBuffers[1]);

	// Alternately...
	// GLuint   vaoID;
	// glGenVertexArrays(1, &vaoID);
	// glBindVertexArray(vaoID);
	//


	glGenBuffers(1, &(arrayBuffers[1]));
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
	delete[] normals;
	delete[] vertices;
	vertices = readOBJFile("tiger.obj", nbrTriangles[1], normals);
	verticesSize = nbrTriangles[1] * nbrVerticesPerTriangle * nbrFloatsPerVertex * sizeof(GLfloat);
	normalsSize = nbrTriangles[1] * nbrVerticesPerTriangle * nbrFloatsPerNormal * sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, verticesSize + normalsSize,
		NULL, GL_STATIC_DRAW);
	//                               offset in bytes   size in bytes     ptr to data    
	glBufferSubData(GL_ARRAY_BUFFER, 0, verticesSize, vertices);
	glBufferSubData(GL_ARRAY_BUFFER, verticesSize, normalsSize, normals);
	/*
	 * Set up variables into the shader programs (Note:  We need the
	 * shaders loaded and built into a program before we do this)
	 */
	vPosition = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	vNormal = glGetAttribLocation(programID, "vNormal");
	if (vNormal != -1) {
		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(verticesSize));
	}
	/////////////////////////////////////////////////////////
	///						Landon Edit					 ///
	///////////////////////////////////////////////////////

	  ///////////////////////////////////////////////////////
	 ///						Roof					 ///
	///////////////////////////////////////////////////////

	glGenVertexArrays(1, &vertexBuffers[2]);
	glBindVertexArray(vertexBuffers[2]);

	// Alternately...
	// GLuint   vaoID;
	// glGenVertexArrays(1, &vaoID);
	// glBindVertexArray(vaoID);
	//


	glGenBuffers(1, &(arrayBuffers[2]));
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[2]);
	delete[] normals;
	delete[] vertices;
	vertices = readOBJFile("roof.obj", nbrTriangles[2], normals);
	verticesSize = nbrTriangles[2] * nbrVerticesPerTriangle * nbrFloatsPerVertex * sizeof(GLfloat);
	normalsSize = nbrTriangles[2] * nbrVerticesPerTriangle * nbrFloatsPerNormal * sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, verticesSize + normalsSize,
		NULL, GL_STATIC_DRAW);
	//                               offset in bytes   size in bytes     ptr to data    
	glBufferSubData(GL_ARRAY_BUFFER, 0, verticesSize, vertices);
	glBufferSubData(GL_ARRAY_BUFFER, verticesSize, normalsSize, normals);
	/*
	 * Set up variables into the shader programs (Note:  We need the
	 * shaders loaded and built into a program before we do this)
	 */
	vPosition = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	vNormal = glGetAttribLocation(programID, "vNormal");
	if (vNormal != -1) {
		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(verticesSize));
	}

	//col2

	glGenVertexArrays(1, &vertexBuffers[3]);
	glBindVertexArray(vertexBuffers[3]);

	// Alternately...
	// GLuint   vaoID;
	// glGenVertexArrays(1, &vaoID);
	// glBindVertexArray(vaoID);
	//


	glGenBuffers(1, &(arrayBuffers[3]));
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[3]);
	delete[] normals;
	delete[] vertices;
	vertices = readOBJFile("column.obj", nbrTriangles[3], normals);
	verticesSize = nbrTriangles[3] * nbrVerticesPerTriangle * nbrFloatsPerVertex * sizeof(GLfloat);
	normalsSize = nbrTriangles[3] * nbrVerticesPerTriangle * nbrFloatsPerNormal * sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, verticesSize + normalsSize,
		NULL, GL_STATIC_DRAW);
	//                               offset in bytes   size in bytes     ptr to data    
	glBufferSubData(GL_ARRAY_BUFFER, 0, verticesSize, vertices);
	glBufferSubData(GL_ARRAY_BUFFER, verticesSize, normalsSize, normals);
	/*
	 * Set up variables into the shader programs (Note:  We need the
	 * shaders loaded and built into a program before we do this)
	 */
	vPosition = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	vNormal = glGetAttribLocation(programID, "vNormal");
	if (vNormal != -1) {
		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(verticesSize));
	}

	//col3

	glGenVertexArrays(1, &vertexBuffers[4]);
	glBindVertexArray(vertexBuffers[4]);

	// Alternately...
	// GLuint   vaoID;
	// glGenVertexArrays(1, &vaoID);
	// glBindVertexArray(vaoID);
	//


	glGenBuffers(1, &(arrayBuffers[4]));
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[4]);
	delete[] normals;
	delete[] vertices;
	vertices = readOBJFile("column.obj", nbrTriangles[4], normals);
	verticesSize = nbrTriangles[4] * nbrVerticesPerTriangle * nbrFloatsPerVertex * sizeof(GLfloat);
	normalsSize = nbrTriangles[4] * nbrVerticesPerTriangle * nbrFloatsPerNormal * sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, verticesSize + normalsSize,
		NULL, GL_STATIC_DRAW);
	//                               offset in bytes   size in bytes     ptr to data    
	glBufferSubData(GL_ARRAY_BUFFER, 0, verticesSize, vertices);
	glBufferSubData(GL_ARRAY_BUFFER, verticesSize, normalsSize, normals);
	/*
	 * Set up variables into the shader programs (Note:  We need the
	 * shaders loaded and built into a program before we do this)
	 */
	vPosition = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	vNormal = glGetAttribLocation(programID, "vNormal");
	if (vNormal != -1) {
		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(verticesSize));
	}

	//col4

	glGenVertexArrays(1, &vertexBuffers[5]);
	glBindVertexArray(vertexBuffers[5]);

	// Alternately...
	// GLuint   vaoID;
	// glGenVertexArrays(1, &vaoID);
	// glBindVertexArray(vaoID);
	//


	glGenBuffers(1, &(arrayBuffers[5]));
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[5]);
	delete[] normals;
	delete[] vertices;
	vertices = readOBJFile("column.obj", nbrTriangles[5], normals);
	verticesSize = nbrTriangles[5] * nbrVerticesPerTriangle * nbrFloatsPerVertex * sizeof(GLfloat);
	normalsSize = nbrTriangles[5] * nbrVerticesPerTriangle * nbrFloatsPerNormal * sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, verticesSize + normalsSize,
		NULL, GL_STATIC_DRAW);
	//                               offset in bytes   size in bytes     ptr to data    
	glBufferSubData(GL_ARRAY_BUFFER, 0, verticesSize, vertices);
	glBufferSubData(GL_ARRAY_BUFFER, verticesSize, normalsSize, normals);
	/*
	 * Set up variables into the shader programs (Note:  We need the
	 * shaders loaded and built into a program before we do this)
	 */
	vPosition = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	vNormal = glGetAttribLocation(programID, "vNormal");
	if (vNormal != -1) {
		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(verticesSize));
	}

}

/*
 * This fills in the locations of most of the uniform variables for the program.
 * there are better ways of handling this but this is good in going directly from
 * what we had.
 *
 * Revised to get the locations and names of the uniforms from OpenGL.  These
 * are then stored in a map so that we can look up a uniform by name when we
 * need to use it.  The map is still global but it is a little neater than the
 * version that used all the locations.  The locations are still there right now
 * in case that is more useful for you.
 *
 */

void getLocations() {
	/*
	 * Find out how many uniforms there are and go out there and get them from the
	 * shader program.  The locations for each uniform are stored in a global -- locationMap --
	 * for later retrieval.
	 */
	GLint numberBlocks;
	char uniformName[1024];
	int nameLength;
	GLint size;
	GLenum type;
	glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &numberBlocks);
	for (int blockIndex = 0; blockIndex < numberBlocks; blockIndex++) {
		glGetActiveUniform(programID, blockIndex, 1024, &nameLength, &size, &type, uniformName);
		cout << uniformName << endl;
		locationMap[string(uniformName)] = blockIndex;
	}
}

void init(string vertexShader, string fragmentShader) {

	setAttributes(1.0f, GL_FRONT_AND_BACK, GL_FILL);

	programID = buildProgram(vertexShader, fragmentShader);
	mat4x4_identity(rotation);

	buildObjects();

	getLocations();
	// Originally set the viewing, projection, and modeling matrices to the identity matrix
	// 
	glUniformMatrix4fv(locationMap["viewingMatrix"], 1, false, (const GLfloat*)rotation);
	glUniformMatrix4fv(locationMap["projectionMatrix"], 1, false, (const GLfloat*)rotation);
	glUniformMatrix4fv(locationMap["modelingMatrix"], 1, false, (const GLfloat*)rotation);
}

/*
 * The display routine is basically unchanged at this point.
 */
void display() {
	mat4x4 translationMatrix;
	mat4x4 translationMatrixCol2;
	mat4x4 translationMatrixRoof;
	mat4x4 identityMatrixTiger;

	mat4x4_identity(identityMatrixTiger);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// needed -- clears screen before drawing. 

	GLuint modelMatrixLocation = glGetUniformLocation(programID, "modelingMatrix");
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)rotation);
	GLuint colorLocation = glGetUniformLocation(programID, "color");
	GLfloat columnColor[] = { 1.0f, 1.0f, 0.0f, 1.0f }; // color to draw with -- currently magenta
	glUniform4fv(colorLocation, 1, columnColor);
	//Draw Column1
	mat4x4_translate(translationMatrix, -0.5f, -0.5f, 0.5f);
	columnColor[0] = 0.0f;
	glBindVertexArray(vertexBuffers[0]);
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)translationMatrix);
	glDrawArrays(GL_TRIANGLES, 0, nbrTriangles[0] * 3);
	//Draw Column2
	columnColor[0] = 0.0f;
	columnColor[1] = 1.0f;
	columnColor[2] = 0.0f;
	columnColor[3] = 1.0f;
	glUniform4fv(colorLocation, 1, columnColor);
	mat4x4_translate(translationMatrixCol2, -0.5f, -0.5f, -0.5f);
	glBindVertexArray(vertexBuffers[3]);
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[3]);
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)translationMatrixCol2);
	glDrawArrays(GL_TRIANGLES, 0, nbrTriangles[0] * 3);
	//glDrawArrays(GL_TRIANGLES, 0, nbrTriangles[3] * 3);
	//Draw Column3
	mat4x4_translate(translationMatrix, 0.5f, -0.5f, 0.5f);
	glBindVertexArray(vertexBuffers[4]);
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[4]);
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)translationMatrix);
	glDrawArrays(GL_TRIANGLES, 0, nbrTriangles[0] * 3);
	//glDrawArrays(GL_TRIANGLES, 0, nbrTriangles[4] * 3);
	//Draw Column4
	mat4x4_translate(translationMatrix, 0.5f, -0.5f, -0.5f);
	glBindVertexArray(vertexBuffers[5]);
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[5]);
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)translationMatrix);
	glDrawArrays(GL_TRIANGLES, 0, nbrTriangles[0] * 3);
	//glDrawArrays(GL_TRIANGLES, 0, nbrTriangles[5] * 3);

	//  draw a tiger...
	mat4x4_translate(identityMatrixTiger, 0.0f, -0.5f, 0.0f);
	glBindVertexArray(vertexBuffers[1]);
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)identityMatrixTiger);
	glDrawArrays(GL_TRIANGLES, 0, nbrTriangles[1] * 3);

	//Draw Roof
	mat4x4_translate(translationMatrixRoof, 0.0f, 0.5f, 0.0f);
	glBindVertexArray(vertexBuffers[2]);
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[2]);
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)translationMatrixRoof);
	glDrawArrays(GL_TRIANGLES, 0, nbrTriangles[2] * 3);

}

/*
* Handle window resizes -- adjust size of the viewport -- more on this later
*/

void reshapeWindow(GLFWwindow* window, int width, int height)
{
	float ratio;
	ratio = width / (float)height;

	glViewport(0, 0, width, height);

}
/*
* Main program with calls for many of the helper routines.
*/
int main(int argCount, char* argValues[]) {
	GLFWwindow* window = nullptr;
	window = glfwStartUp(argCount, argValues, "Project 1 : Landon Jackson");
	init("project1.vert", "project1.frag");
	glfwSetWindowSizeCallback(window, reshapeWindow);

	while (!glfwWindowShouldClose(window))
	{
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	};

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}