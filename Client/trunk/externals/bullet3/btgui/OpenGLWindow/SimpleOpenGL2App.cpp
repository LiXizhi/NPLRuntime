#include "OpenGLWindow/SimpleOpenGL2App.h"
#include "OpenGLWindow/OpenGLInclude.h"

#include "Bullet3Common/b3Logging.h"//b3Assert?
#include "Bullet3Common/b3Scalar.h"
#include "Bullet3Common/b3AlignedObjectArray.h"
#include "Bullet3Common/b3Vector3.h"


#include "stdlib.h"

#ifdef __APPLE__
#include "OpenGLWindow/MacOpenGLWindow.h"
#else

#include "OpenGLWindow/GlewWindows/GL/glew.h"
//#include "GL/glew.h"
#ifdef _WIN32
#include "OpenGLWindow/Win32OpenGLWindow.h"
#else
//let's cross the fingers it is Linux/X11
#include "OpenGLWindow/X11OpenGLWindow.h"
#endif //_WIN32
#endif//__APPLE__
#include <stdio.h>
#include "OpenGLWindow/CommonRenderInterface.h"

struct SimpleOpenGL2AppInternalData
{

};

SimpleOpenGL2App::SimpleOpenGL2App(const char* title, int width, int height)
{
	m_data = new SimpleOpenGL2AppInternalData;

	m_window = new b3gDefaultOpenGLWindow();
	b3gWindowConstructionInfo ci;
	ci.m_title = title;
	ci.m_openglVersion = 2;
	ci.m_width = width;
	ci.m_height = height;
	m_window->createWindow(ci);

	m_window->setWindowTitle(title);

#ifndef __APPLE__
#ifndef _WIN32
//some Linux implementations need the 'glewExperimental' to be true
    glewExperimental = GL_TRUE;
#endif


    if (glewInit() != GLEW_OK)
	{
        b3Error("glewInit failed");
		exit(1);
	}
    if (!GLEW_VERSION_2_1)  // check that the machine supports the 2.1 API.
	{
		b3Error("GLEW_VERSION_2_1 needs to support 2_1");
		exit(1); // or handle the error in a nicer way
	}

#endif
    glGetError();//don't remove this call, it is needed for Ubuntu
	glClearColor(0.9,0.9,1,1);

    b3Assert(glGetError() ==GL_NO_ERROR);

	//m_primRenderer = new GLPrimitiveRenderer(width,height);
	m_parameterInterface = 0;

    b3Assert(glGetError() ==GL_NO_ERROR);

	//m_instancingRenderer = new GLInstancingRenderer(128*1024,32*1024*1024);
	//m_instancingRenderer->init();
	//m_instancingRenderer->resize(width,height);

	b3Assert(glGetError() ==GL_NO_ERROR);

	//m_instancingRenderer->InitShaders();

}

SimpleOpenGL2App::~SimpleOpenGL2App()
{
	delete m_data;
}

void SimpleOpenGL2App::drawGrid(DrawGridData data)
{
	 int gridSize = data.gridSize;
    float upOffset = data.upOffset;
    int upAxis = data.upAxis;
    float gridColor[4];
    gridColor[0] = data.gridColor[0];
    gridColor[1] = data.gridColor[1];
    gridColor[2] = data.gridColor[2];
    gridColor[3] = data.gridColor[3];

	int sideAxis=-1;
	int forwardAxis=-1;

	switch (upAxis)
	{
		case 1:
			forwardAxis=2;
			sideAxis=0;
			break;
		case 2:
			forwardAxis=1;
			sideAxis=0;
			break;
		default:
			b3Assert(0);
	};
	//b3Vector3 gridColor = b3MakeVector3(0.5,0.5,0.5);

	 b3AlignedObjectArray<unsigned int> indices;
		 b3AlignedObjectArray<b3Vector3> vertices;
	int lineIndex=0;
	for(int i=-gridSize;i<=gridSize;i++)
	{
		{
			b3Assert(glGetError() ==GL_NO_ERROR);
			b3Vector3 from = b3MakeVector3(0,0,0);
			from[sideAxis] = float(i);
			from[upAxis] = upOffset;
			from[forwardAxis] = float(-gridSize);
			b3Vector3 to=b3MakeVector3(0,0,0);
			to[sideAxis] = float(i);
			to[upAxis] = upOffset;
			to[forwardAxis] = float(gridSize);
			vertices.push_back(from);
			indices.push_back(lineIndex++);
			vertices.push_back(to);
			indices.push_back(lineIndex++);
		//	m_renderer->drawLine(from,to,gridColor);
		}

		b3Assert(glGetError() ==GL_NO_ERROR);
		{

			b3Assert(glGetError() ==GL_NO_ERROR);
			b3Vector3 from=b3MakeVector3(0,0,0);
			from[sideAxis] = float(-gridSize);
			from[upAxis] = upOffset;
			from[forwardAxis] = float(i);
			b3Vector3 to=b3MakeVector3(0,0,0);
			to[sideAxis] = float(gridSize);
			to[upAxis] = upOffset;
			to[forwardAxis] = float(i);
			vertices.push_back(from);
			indices.push_back(lineIndex++);
			vertices.push_back(to);
			indices.push_back(lineIndex++);
		//	m_renderer->drawLine(from,to,gridColor);
		}

	}


	m_renderer->drawLines(&vertices[0].x,
			gridColor,
			vertices.size(),sizeof(b3Vector3),&indices[0],indices.size(),1);
	

	m_renderer->drawLine(b3MakeVector3(0,0,0),b3MakeVector3(1,0,0),b3MakeVector3(1,0,0),3);
	m_renderer->drawLine(b3MakeVector3(0,0,0),b3MakeVector3(0,1,0),b3MakeVector3(0,1,0),3);
	m_renderer->drawLine(b3MakeVector3(0,0,0),b3MakeVector3(0,0,1),b3MakeVector3(0,0,1),3);

//	void GLInstancingRenderer::drawPoints(const float* positions, const float color[4], int numPoints, int pointStrideInBytes, float pointDrawSize)

	//we don't use drawPoints because all points would have the same color
//	b3Vector3 points[3] = { b3MakeVector3(1, 0, 0), b3MakeVector3(0, 1, 0), b3MakeVector3(0, 0, 1) };
//	m_instancingRenderer->drawPoints(&points[0].x, b3MakeVector3(1, 0, 0), 3, sizeof(b3Vector3), 6);
}
void SimpleOpenGL2App::setUpAxis(int axis)
{
}
int SimpleOpenGL2App::getUpAxis() const
{
	return 1;
}
	
void SimpleOpenGL2App::swapBuffer()
{
	m_window->endRendering();
	m_window->startRendering();

}
void SimpleOpenGL2App::drawText( const char* txt, int posX, int posY)
{

}

void SimpleOpenGL2App::drawText3D( const char* txt, float posX, float posZY, float posZ, float size)
{

}

void SimpleOpenGL2App::registerGrid(int xres, int yres, float color0[4], float color1[4])
{
    
}

