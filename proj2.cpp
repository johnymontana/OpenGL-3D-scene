// proj2.cpp
// William Lyon
// CSCI 441
// Project 2
// 12.13.12
// 3D scene - giant golf ball with orbiting ball of fire, textured trees and terrain
// adapted from SphereWorld demo found in OpenGL SuperBible 4th Edition
// uses gltools, math3d and glframe libraries provided by OpenGL SuperBible 4th Edition
// and objLoader library provided by kixor.net
// model_obj2mesh function written by William Knight
//
// Golf ball and tree models were created in Blender and exported to .obj files by William Lyon
//
#include "gltools.h"	// OpenGL toolkit
#include "math3d.h"    // 3D Math Library
#include "glframe.h"   // Frame class
#include "objLoader.h"

#include <math.h>
#include <string.h>
#define NUM_TREES     30
#define GLUT_KEY_ESC 27
#define GLUT_KEY_h 104
#define GLUT_KEY_H 48

GLFrame    trees[NUM_TREES];        // trees drawn randomly throughout scene
GLFrame    frameCamera;             // keep track of frame for camera

typedef float Real;
unsigned FrameCount = 0;        // count frames drawn
bool drawHelp = false;

struct Mesh {
	int mesh_created;
	Real* verts;
	Real* normals;
	Real* tcoords;
	GLuint* tris;
	GLsizei num_tris;
	GLsizei num_verts;
    GLsizei num_tcoords;
	GLuint texture_id;
	// vbo's
	int buf_created;
	GLuint vert_buf;
	GLuint normal_buf;
	GLuint tcoord_buf;
	GLuint index_buf;
};
typedef struct Mesh Mesh;

Mesh golf_ball_mesh;
Mesh tree_mesh;
int vert_count;         // count number of vertices being drawn

void renderBitmapString(        // draw text
		float x,
		float y,
		float z,
		void *font,
		char *string) {

  char *c;
  glRasterPos3f(x, y, z);
  for (c=string; *c != '\0'; c++) {
    glutBitmapCharacter(font, *c);
  }
}


void mesh_draw (float* verts, GLuint* tris, int num_tris)
{
	Real normalVec[] = {0.0, 0.0, 0.0};
    //vert_count = 0;
    glBegin (GL_TRIANGLES);
	int i;
	for (i=0; i<num_tris; i++)
	{
		//int tri = tri+i*3;

		float* v0 = verts + tris[i*3+0]*3;
		float* v1 = verts + tris[i*3+1]*3;
		float* v2 = verts + tris[i*3+2]*3;

		//geom_calc_normal(v0, v1, v2, normalVec);

        m3dFindNormal(normalVec, v0, v1, v2);
        m3dNormalizeVector(normalVec);
        glNormal3f(normalVec[0], normalVec[1], normalVec[2]);

        glTexCoord2f(0,0);
        glVertex3f(v0[0], v0[1], v0[2]);
        glTexCoord2f(0,1);
		glVertex3f(v1[0], v1[1], v1[2]);
		glTexCoord2f(1,0);
        glVertex3f(v2[0], v2[1], v2[2]);
        glTexCoord2f(1,1);
        vert_count += 3;

    }

    glEnd();
    glFlush();
}

/*
 descrip: Converts tris in param obj to param mesh object.

 params:
 obj : source obj model data (can be created with objloader_load())

 mesh_ret : mesh struct that will receive the geometry and texture from conversion

 err_txt_ret : can be NULL, if not, it should be at least 1024 in size and will receive
 error description if an error occurs

 returns: 1 if success, else 0

 notes:
 * currently, the obj model must ONLY have faces with 3 verts

 * Also, each vertex position should be associated with a unique texture and normal.
 If the same vertex position is associated with multiple textures/normals in
 face points, only the most recent vertex/texture/normal triplet will be used.
 The 'singulate_obj_verts.lua script can be used to de-multiplex verts in an obj file.

 * currently, material data in obj->material_list is not used, except for
 obj->material_list[0]->texture_filename, which if not NULL will be used to
 create a single texture for the mesh
 */
int model_obj2mesh(const struct obj_scene_data* obj, struct Mesh* mesh_ret, char* err_txt_ret) {
	struct Mesh* m = mesh_ret;
    memset(mesh_ret, 0, sizeof(struct Mesh));

    // check whether tcoords are defined for all triangles
    // count the number of faces that are triangles
	int i;
	int tri_face_count = 0;
	for(i = 0; i < obj->face_count; ++i) {
		obj_face* face = obj->face_list[i];
		// just skip non-tri faces for now
		if(face->vertex_count != 3) {
			continue;
		}
		++tri_face_count;
	}
	m->num_tris = tri_face_count;

    // allocate vertex, tcoord, normal and tri arrays
	size_t n;
	n = sizeof(Real) * 3 * obj->vertex_count;
	m->verts = (Real*) malloc(n);
	memset(m->verts, 0, n);
	m->num_verts = obj->vertex_count;

	n = sizeof(GLuint) * 3 * tri_face_count;
	m->tris = (GLuint*) malloc(n);
	memset(m->tris, 0, n);

    // iterate all faces, get mesh data for tri faces
	for(i = 0; i < obj->face_count; ++i) {
		obj_face* face = obj->face_list[i];
		if(face->vertex_count != 3)
			continue;

		int j;
		int fv[3];
		for(j = 0; j < 3; ++j) {
			fv[j] = face->vertex_index[j];
		}

		int b;

        // verts
		double* ov0 = obj->vertex_list[fv[0]]->e;
		double* ov1 = obj->vertex_list[fv[1]]->e;
		double* ov2 = obj->vertex_list[fv[2]]->e;

		b = fv[0]*3;
		m->verts[b    ] = ov0[0];
		m->verts[b + 1] = ov0[1];
		m->verts[b + 2] = ov0[2];
		Real* v0 = m->verts + b;

		b = fv[1]*3;
		m->verts[b    ] = ov1[0];
		m->verts[b + 1] = ov1[1];
		m->verts[b + 2] = ov1[2];
		Real* v1 = m->verts + b;

		b = fv[2]*3;
		m->verts[b    ] = ov2[0];
		m->verts[b + 1] = ov2[1];
		m->verts[b + 2] = ov2[2];
		Real* v2 = m->verts + b;

		// copy indices
		m->tris[i*3  ] = fv[0];
		m->tris[i*3+1] = fv[1];
		m->tris[i*3+2] = fv[2];
	}
	m->mesh_created = 1;

	return 1;
}


GLfloat fLightPos[4]   = { -100.0f, 100.0f, 50.0f, 0.0f };  // Point source
GLfloat fNoLight[] = { 0.0f, 0.0f, 0.0f, 0.0f };
GLfloat fLowLight[] = { 0.25f, 0.25f, 0.25f, 1.0f };
GLfloat fBrightLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };

M3DMatrix44f mShadowMatrix;

#define GROUND_TEXTURE  0
#define TORUS_TEXTURE   1
#define SPHERE_TEXTURE  2
#define NUM_TEXTURES    3
GLuint  textureObjects[NUM_TEXTURES];

const char *szTextureFiles[] = {"grass.tga", "wood.tga", "orb.tga"};

void SetupRC()
    {
    M3DVector3f vPoints[3] = {{ 0.0f, -0.4f, 0.0f },
                             { 10.0f, -0.4f, 0.0f },
                             { 5.0f, -0.4f, -5.0f }};
    int iTree;
    int i;

    // Grayish background
    glClearColor(fLowLight[0], fLowLight[1], fLowLight[2], fLowLight[3]);

    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    glClearStencil(0);
    glStencilFunc(GL_EQUAL, 0x0, 0x01);

    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE_ARB);

    // Setup light parameters
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, fNoLight);
    glLightfv(GL_LIGHT0, GL_AMBIENT, fLowLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, fBrightLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, fBrightLight);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat light1_position[] = { 1.0, 1.0, 1.0, 0.0 };
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glEnable(GL_LIGHT1);

    // Calculate shadow matrix
    M3DVector4f pPlane;
    m3dGetPlaneEquation(pPlane, vPoints[0], vPoints[1], vPoints[2]);
    m3dMakePlanarShadowMatrix(mShadowMatrix, pPlane, fLightPos);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv(GL_FRONT, GL_SPECULAR, fBrightLight);
    glMateriali(GL_FRONT, GL_SHININESS, 128);

    // draw trees in random location
    for(iTree = 0; iTree < NUM_TREES; iTree++)
        {
        // Pick a random location between -20 and 20 at .1 increments
        trees[iTree].SetOrigin(((float)((rand() % 400) - 200) * 0.1f), 0.0, (float)((rand() % 400) - 200) * 0.1f);
        }

    // Set up texture maps
    glEnable(GL_TEXTURE_2D);
    glGenTextures(NUM_TEXTURES, textureObjects);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


    for(i = 0; i < NUM_TEXTURES; i++)
        {
        GLbyte *pBytes;
        GLint iWidth, iHeight, iComponents;
        GLenum eFormat;

        glBindTexture(GL_TEXTURE_2D, textureObjects[i]);

        // Load this texture map
        pBytes = gltLoadTGA(szTextureFiles[i], &iWidth, &iHeight, &iComponents, &eFormat);
        gluBuild2DMipmaps(GL_TEXTURE_2D, iComponents, iWidth, iHeight, eFormat, GL_UNSIGNED_BYTE, pBytes);
        free(pBytes);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

    }

////////////////////////////////////////////////////////////////////////
// Do shutdown for the rendering context
void ShutdownRC(void)
    {
    // Delete the textures
    glDeleteTextures(NUM_TEXTURES, textureObjects);
    }


///////////////////////////////////////////////////////////
// Draw the ground as a series of triangle strips
void DrawGround(void)
    {
    GLfloat fExtent = 20.0f;
    GLfloat fStep = 1.0f;
    GLfloat y = -0.4f;
    GLfloat iStrip, iRun;
    GLfloat s = 0.0f;
    GLfloat t = 0.0f;
    GLfloat texStep = 1.0f / (fExtent * .075f);

    glBindTexture(GL_TEXTURE_2D, textureObjects[GROUND_TEXTURE]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    for(iStrip = -fExtent; iStrip <= fExtent; iStrip += fStep)
        {
        t = 0.0f;
        glBegin(GL_TRIANGLE_STRIP);

            for(iRun = fExtent; iRun >= -fExtent; iRun -= fStep)
                {
                glTexCoord2f(s, t);
                glNormal3f(0.0f, 1.0f, 0.0f);   // All Point up
                glVertex3f(iStrip, y, iRun);

                glTexCoord2f(s + texStep, t);
                glNormal3f(0.0f, 1.0f, 0.0f);   // All Point up
                glVertex3f(iStrip + fStep, y, iRun);
                vert_count += 2;
                t += texStep;
                }
        glEnd();
        s += texStep;
        }
    }

void DrawInhabitants(GLint nShadow)
    {
    static GLfloat yRot = 0.0f;         // Rotation angle for animation
    GLint i;

    if(nShadow == 0)
        {
        yRot += 0.5f;
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
    else
        glColor4f(0.00f, 0.00f, 0.00f, .6f);  // Shadow color


    // Draw the randomly located trees
    glBindTexture(GL_TEXTURE_2D, textureObjects[SPHERE_TEXTURE]);
    for(i = 0; i < NUM_TREES; i++)
        {
        glPushMatrix();
        trees[i].ApplyActorTransform();

            mesh_draw(tree_mesh.verts, tree_mesh.tris, tree_mesh.num_tris);

        glPopMatrix();
        }

    glPushMatrix();
        glTranslatef(0.0f, 0.1f, -2.5f);

        glPushMatrix();
            glRotatef(-yRot * 2.0f, 0.0f, 1.0f, 0.0f);
            glTranslatef(1.0f, 0.0f, 0.0f);
            gltDrawSphere(0.1f,21, 11);
        glPopMatrix();

        if(nShadow == 0)
            {
            glMaterialfv(GL_FRONT, GL_SPECULAR, fBrightLight);
            }

        glRotatef(yRot, 0.0f, 1.0f, 0.0f);
        glBindTexture(GL_TEXTURE_2D, textureObjects[TORUS_TEXTURE]);
         mesh_draw(golf_ball_mesh.verts, golf_ball_mesh.tris, golf_ball_mesh.num_tris);
        glMaterialfv(GL_FRONT, GL_SPECULAR, fNoLight);
    glPopMatrix();
    }

static void check_gl_errors() {
    GLuint err;
    while((err=glGetError())!=GL_NO_ERROR) {
        printf("GL ERROR: %s\n", gluErrorString(err));
    }
}

// Called to draw scene
void RenderScene(void)
    {
    // Clear the window with current clearing color
   // vert_count = 0;
        FrameCount++;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	glPushMatrix();
	glLoadIdentity();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    //char* stringToRender;
    char vert_text[64] = "Toggle (h) for help.   Vertices: ";
    char numStr[32];
    char frame_text[64] = "  FPS: ";
    char frameStr[32];

        //vert_text = "Vertices: ";
        sprintf(numStr, "%d", vert_count);
        sprintf(frameStr, "%d", FrameCount*4); // timerfunc is called 4 times per second
        strcat(vert_text, numStr);
        strcat(frame_text, frameStr);
        strcat(vert_text, frame_text);
        glColor3f(1.0f, 0.0f, 0.0f);
        renderBitmapString(-0.1 , 0.3, -1.0, GLUT_BITMAP_9_BY_15, vert_text );
        vert_count=0;

    if (drawHelp==true)
    {
        renderBitmapString(-0.4, 0.275, -1.0, GLUT_BITMAP_9_BY_15, "Willz Golf Courze v1.0 By William Lyon.");
        renderBitmapString(-0.4, 0.25, -1.0, GLUT_BITMAP_9_BY_15, "Use arrow keys to move, move mouse to change view.");

    }

        glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
           // Do the buffer Swap
    glPopMatrix();
	glPopAttrib();

    glPushMatrix();
        frameCamera.ApplyCameraTransform();

        // Position light before any other transformations
        glLightfv(GL_LIGHT0, GL_POSITION, fLightPos);

        // Draw the ground
        glColor3f(1.0f, 1.0f, 1.0f);
       DrawGround();

        // Draw shadows first
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_STENCIL_TEST);
        glPushMatrix();
            glMultMatrixf(mShadowMatrix);
           DrawInhabitants(1);
        glPopMatrix();
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);

        // Draw inhabitants normally
       DrawInhabitants(0);

    glPopMatrix();
     glDisable(GL_LIGHTING);
     glDisable(GL_TEXTURE_2D);
      glDisable(GL_DEPTH_TEST);
  //  set_ortho_projection();


    glutSwapBuffers();
    check_gl_errors();
    }


void MouseFunc(int x, int y)        // handle mouse input to rotate view up/down and right/left
{
    if (x> 400)
    {
        frameCamera.RotateLocalY(-0.01);
    }

    if (x<400)
    {
       frameCamera.RotateLocalY(0.01);
    }

    if (y<300)
    {
       frameCamera.RotateLocalX(-0.01);
    }

    if (y>300)
    {
       frameCamera.RotateLocalX(0.01);
    }
    printf ("x: %d", x);
    printf ("y: %d", y);

}

// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key, int x, int y)
    {
    if(key == GLUT_KEY_UP)
        frameCamera.MoveForward(0.1f);

    if(key == GLUT_KEY_DOWN)
		frameCamera.MoveForward(-0.1f);

    if(key == GLUT_KEY_LEFT)
		frameCamera.MoveRight(0.1f);

    if(key == GLUT_KEY_RIGHT)
        frameCamera.MoveRight(-0.1f);

        // Refresh the Window
    glutPostRedisplay();
    }

void Keys(unsigned char key, int x, int y)      // h/H for help text, ESC to quit
{
    if (key == GLUT_KEY_h || key == GLUT_KEY_H )
        if (drawHelp)
            drawHelp=false;
        else
            drawHelp=true;

    if (key == GLUT_KEY_ESC)
        exit(0);

        glutPostRedisplay();
}

void TimerFunction(int value)
    {
    // Redraw the scene with new coordinates
    glutPostRedisplay();
    glutTimerFunc(3,TimerFunction, 1);
    FrameCount = 0;
    }

void ChangeSize(int w, int h)
    {
    GLfloat fAspect;

    if(h == 0)
        h = 1;

    glViewport(0, 0, w, h);

    fAspect = (GLfloat)w / (GLfloat)h;

    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Set the clipping volume
    gluPerspective(35.0f, fAspect, 1.0f, 50.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    }

int main(int argc, char* argv[])
    {

        obj_scene_data golf_ball_data;
        if (!parse_obj_scene(&golf_ball_data, "golfBall.obj"))
            return 0;

        printf("Number of vertices: %i\n", golf_ball_data.vertex_count);

        obj_scene_data tree_data;

        if (!parse_obj_scene(&tree_data, "tree.obj"))
            return 0;

        char* errorText;

        model_obj2mesh(&golf_ball_data, &golf_ball_mesh, errorText);
        model_obj2mesh(&tree_data, &tree_mesh, errorText);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800,600);
    glutCreateWindow("Willz Golf Courze");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);
    glutPassiveMotionFunc(MouseFunc);
    glutKeyboardFunc(Keys);

    SetupRC();
    glutTimerFunc(250, TimerFunction, 1);

    glutMainLoop();

    ShutdownRC();

    return 0;
    }

