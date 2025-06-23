///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ,
	glm::vec3 offset)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ + offset);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

/***********************************************************
  *  LoadSceneTextures()
  *
  *  This method is used for preparing the 3D scene by loading
  *  the shapes, textures in memory to support the 3D scene
  *  rendering
  ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	/*** STUDENTS - add the code BELOW for loading the textures that ***/
	/*** will be used for mapping to objects in the 3D scene. Up to  ***/
	/*** 16 textures can be loaded per scene. Refer to the code in   ***/
	/*** the OpenGL Sample for help.                                 ***/
	bool bReturn = false;

	bReturn = CreateGLTexture(
		"textures/Grass.jpg",
		"Grass");

	bReturn = CreateGLTexture(
		"textures/Brick.jpg",
		"Brick");

	bReturn = CreateGLTexture(
		"textures/Roof.jpg",
		"Roof");

	bReturn = CreateGLTexture(
		"textures/Wood.jpg",
		"Wood");

	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

void SceneManager::DefineObjectMaterials()
{
	OBJECT_MATERIAL metal;
	metal.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
	metal.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
	metal.shininess = 52.0;
	metal.tag = "metal";

	m_objectMaterials.push_back(metal);

	OBJECT_MATERIAL wood;
	wood.diffuseColor = glm::vec3(0.78f, 0.49f, 0.2f);
	wood.specularColor = glm::vec3(0.96f, 0.72f, 0.49f);
	wood.shininess = 4.0;
	wood.tag = "wood";

	m_objectMaterials.push_back(wood);

	OBJECT_MATERIAL glass;
	glass.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
	glass.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glass.shininess = 0.25;
	glass.tag = "glass";

	m_objectMaterials.push_back(glass);

	OBJECT_MATERIAL stone;
	stone.diffuseColor = glm::vec3(0.7f, 0.7f, 0.7f);
	stone.specularColor = glm::vec3(0.75f, 0.75f, 0.75f);
	stone.shininess = 2.0;
	stone.tag = "stone";

	m_objectMaterials.push_back(stone);

	OBJECT_MATERIAL roof;
	roof.diffuseColor = glm::vec3(0.7f, 0.35f, 0.0f);
	roof.specularColor = glm::vec3(0.9f, 0.53f, 0.17f);
	roof.shininess = 30.0;
	roof.tag = "roof";

	m_objectMaterials.push_back(roof);

	OBJECT_MATERIAL cement;
	cement.diffuseColor = glm::vec3(0.6f, 0.6f, 0.6f);
	cement.specularColor = glm::vec3(0.75f, 0.75f, 0.75f);
	cement.shininess = 1.0;
	cement.tag = "cement";

	m_objectMaterials.push_back(cement);

	OBJECT_MATERIAL grass;
	grass.diffuseColor = glm::vec3(0.7f, 0.8f, 0.2f);
	grass.specularColor = glm::vec3(0.88f, 1.0f, 0.24f);
	grass.shininess = 5.0;
	grass.tag = "grass";

	m_objectMaterials.push_back(grass);

}



/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	// this line of code is NEEDED for telling the shaders to render 
	// the 3D scene with custom lighting - to use the default rendered 
	// lighting then comment out the following line
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// directional light to simulate sunlight
	m_pShaderManager->setVec3Value("directionalLight.direction", 20.0f, -1.0f, -15.0f);
	m_pShaderManager->setVec3Value("directionalLight.ambient", 0.2f, 0.2f, 0.2f);
	m_pShaderManager->setVec3Value("directionalLight.diffuse", 0.8f, 0.8f, 0.9f);
	m_pShaderManager->setVec3Value("directionalLight.specular", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setBoolValue("directionalLight.bActive", true);

	//point light
	m_pShaderManager->setVec3Value("pointLights[0].position", 12.0f, 1.0f, -4.0f);
	m_pShaderManager->setVec3Value("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", 0.33f, 0.14f, 0.02f);
	m_pShaderManager->setVec3Value("pointLights[0].specular", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);

	//point light
	m_pShaderManager->setVec3Value("pointLights[1].position", 11.0f, 1.0f, -8.0f);
	m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.33f, 0.14f, 0.02f);
	m_pShaderManager->setVec3Value("pointLights[1].specular", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setBoolValue("pointLights[1].bActive", true);

	//point light
	m_pShaderManager->setVec3Value("pointLights[2].position", -4.0f, 1.0f, -10.0f);
	m_pShaderManager->setVec3Value("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[2].diffuse", 0.33f, 0.14f, 0.02f);
	m_pShaderManager->setVec3Value("pointLights[2].specular", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setBoolValue("pointLights[2].bActive", true);

	//point light
	m_pShaderManager->setVec3Value("pointLights[3].position", 8.5f, 1.0f, -10.0f);
	m_pShaderManager->setVec3Value("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[3].diffuse", 0.33f, 0.14f, 0.02f);
	m_pShaderManager->setVec3Value("pointLights[3].specular", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setBoolValue("pointLights[3].bActive", true);

}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{

	// load the textures for the 3D scene
	LoadSceneTextures();
	// define the materials that will be used for the objects
	// in the 3D scene
	DefineObjectMaterials();
	// add and defile the light sources for the 3D scene
	SetupSceneLights();

	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadPrismMesh();
	m_basicMeshes->LoadPyramid4Mesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	m_basicMeshes->LoadTorusMesh();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	RenderGround(0.0f);
	RenderHouse(0.0f, 0.0f, 0.0, -6.6f, 0.0f, -7.2f);
	RenderHouse(0.0f, 0.0f, 0.0, -4.4f, 0.0f, -7.2f);
	RenderHouse(0.0f, 0.0f, 0.0, -2.2f, 0.0f, -7.2f);
	RenderHouse(0.0f, 0.0f, 0.0, 0.0f, 0.0f, -7.2f);
	RenderHouse(0.0f, 0.0f, 0.0, 2.2f, 0.0f, -7.2f);
	RenderHouse(0.0f, 0.0f, 0.0, 4.4f, 0.0f, -7.2f);
	RenderHouse(0.0f, 0.0f, 0.0, 6.6f, 0.0f, -7.2f);
	RenderHouse(0.0f, 0.0f, 0.0, -6.6f, 0.0f, -3.1f);
	RenderHouse(0.0f, 0.0f, 0.0, -4.4f, 0.0f, -3.1f);
	RenderHouse(0.0f, 0.0f, 0.0, -2.2f, 0.0f, -3.1f);
	RenderHouse(0.0f, 0.0f, 0.0, 0.0f, 0.0f, -3.1f);
	RenderHouse(0.0f, 0.0f, 0.0, 2.2f, 0.0f, -3.1f);
	RenderHouse(0.0f, 0.0f, 0.0, 4.4f, 0.0f, -3.1f);
	RenderHouse(0.0f, 0.0f, 0.0, 6.6f, 0.0f, -3.1f);
	RenderHouse2(0.0f, 0.0f, 0.0, 9.0f, 0.0f, -2.8f);
	RenderHouse2(0.0f, 0.0f, 0.0, 9.0f, 0.0f, -5.0f);
	RenderHouse2(0.0f, 0.0f, 0.0, 9.0f, 0.0f, -7.2f);
	RenderHouse3(0.0f, 0.0f, 0.0, -9.0f, 0.0f, -2.8f);
	RenderHouse3(0.0f, 0.0f, 0.0, -9.0f, 0.0f, -5.0f);
	RenderHouse3(0.0f, 0.0f, 0.0, -9.0f, 0.0f, -7.2f);
	RenderWall();
	RenderWindmill();
}

/***********************************************************
 *  RenderGround()
 *
 *  This method is called to render the shapes for the Ground oject.
 ***********************************************************/
void SceneManager::RenderGround(float Xrotation)
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(50.0f, 1.0f, 30.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = Xrotation;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color for the next draw command
	//SetShaderColor(0.18, 0.34, 0.22, 1.0);
	SetShaderTexture("Grass");
	SetTextureUVScale(16.0, 16.0);
	SetShaderMaterial("grass");
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/
}

/***********************************************************
 *  RenderHouse1()
 *
 *  This method is called to render the shapes for the House object.
 * 
 *  This is the house that is facing toward the camera.
 ***********************************************************/
void SceneManager::RenderHouse(float Xrot, float Yrot, float Zrot, float Xpos, float Ypos, float Zpos)
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;


	/*** Render Building											***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.0f, 1.0f, 2.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f + Xrot;
	YrotationDegrees = 90.0f + Yrot;
	ZrotationDegrees = 0.0f + Zrot;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f + Xpos, 0.5f + Ypos, 0.0f + Zpos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color for the next draw command
	SetShaderColor(0.91, 0.85, 0.71, 1.0);
	SetShaderTexture("Brick");
	SetTextureUVScale(4.0, 4.0);
	SetShaderMaterial("stone");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/*** Render Roof												***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.0f, 2.05f, 1.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = -90.0f + Xrot;
	YrotationDegrees = 90.0f + Yrot;
	ZrotationDegrees = 0.0f + Zrot;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f + Xpos, 1.5f + Ypos, 0.0f + Zpos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color for the next draw command
	//SetShaderColor(0.36, 0.16, 0.11, 1.0);
	SetShaderTexture("Roof");
	SetTextureUVScale(1.25, 2.25);
	SetShaderMaterial("roof");
	// draw the mesh with transformation values
	m_basicMeshes->DrawPrismMesh();
	/****************************************************************/

	/*** Render Door												***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.25f, 0.50f, 0.25f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f + Xrot;
	YrotationDegrees = 0.0f + Yrot;
	ZrotationDegrees = 0.0f + Zrot;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f + Xpos, 0.25f + Ypos, 0.39f + Zpos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	// set the color for the next draw command
	//SetShaderColor(0.32, 0.10, 0.02, 1.0);
	SetShaderTexture("Wood");
	SetTextureUVScale(1.5, 2.0);
	SetShaderMaterial("wood");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/*** Render Window												***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.375f, 0.375f, 0.25f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f + Xrot;
	YrotationDegrees = 0.0f + Yrot;
	ZrotationDegrees = 0.0f + Zrot;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-0.5f + Xpos, 0.625f + Ypos, 0.39f + Zpos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.41, 0.83, 0.85, 1.0);
	SetShaderMaterial("glass");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

		// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/*** Render Window2												***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.375f, 0.375f, 0.25f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f + Xrot;
	YrotationDegrees = 0.0f + Yrot;
	ZrotationDegrees = 0.0f + Zrot;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.5f + Xpos, 0.625f + Ypos, 0.39f + Zpos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.41, 0.83, 0.85, 1.0);
	SetShaderMaterial("glass");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	}

/***********************************************************
 *  RenderHouse2()
 *
 *  This method is called to render the shapes for the House object.
 * 
 *   This is the house that is facing to the left of the camera.
 ***********************************************************/
void SceneManager::RenderHouse2(float Xrot, float Yrot, float Zrot, float Xpos, float Ypos, float Zpos)
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;


	/*** Render Building											***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.0f, 1.0f, 2.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f + Xrot;
	YrotationDegrees = 0.0f + Yrot;
	ZrotationDegrees = 0.0f + Zrot;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f + Xpos, 0.5f + Ypos, 0.0f + Zpos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color for the next draw command
	SetShaderColor(0.91, 0.85, 0.71, 1.0);
	SetShaderTexture("Brick");
	SetTextureUVScale(4.0, 4.0);
	SetShaderMaterial("stone");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/*** Render Roof												***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.0f, 2.05f, 1.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = -90.0f + Xrot;
	YrotationDegrees = 0.0f + Yrot;
	ZrotationDegrees = 0.0f + Zrot;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f + Xpos, 1.5f + Ypos, 0.0f + Zpos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color for the next draw command
	//SetShaderColor(0.36, 0.16, 0.11, 1.0);
	SetShaderTexture("Roof");
	SetTextureUVScale(1.25, 2.25);
	SetShaderMaterial("roof");
	// draw the mesh with transformation values
	m_basicMeshes->DrawPrismMesh();
	/****************************************************************/

	/*** Render Door												***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.25f, 0.50f, 0.25f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f + Xrot;
	YrotationDegrees = 90.0f + Yrot;
	ZrotationDegrees = 0.0f + Zrot;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-0.39f + Xpos, 0.25f + Ypos, 0.0f + Zpos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	// set the color for the next draw command
	//SetShaderColor(0.32, 0.10, 0.02, 1.0);
	SetShaderTexture("Wood");
	SetTextureUVScale(1.5, 2.0);
	SetShaderMaterial("wood");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/*** Render Window												***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.375f, 0.375f, 0.25f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f + Xrot;
	YrotationDegrees = 90.0f + Yrot;
	ZrotationDegrees = 0.0f + Zrot;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-0.39f + Xpos, 0.625f + Ypos, 0.5f + Zpos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.41, 0.83, 0.85, 1.0);
	SetShaderMaterial("glass");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

		// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/*** Render Window2												***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.375f, 0.375f, 0.25f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f + Xrot;
	YrotationDegrees = 90.0f + Yrot;
	ZrotationDegrees = 0.0f + Zrot;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-0.39f + Xpos, 0.625f + Ypos, -0.5f + Zpos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.41, 0.83, 0.85, 1.0);
	SetShaderMaterial("glass");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	}


	/***********************************************************
 *  RenderHouse3()
 *
 *  This method is called to render the shapes for the House object.
 *
 *   This is the house that is facing to the right of the camera.
 ***********************************************************/
	void SceneManager::RenderHouse3(float Xrot, float Yrot, float Zrot, float Xpos, float Ypos, float Zpos)
	{
		// declare the variables for the transformations
		glm::vec3 scaleXYZ;
		float XrotationDegrees = 0.0f;
		float YrotationDegrees = 0.0f;
		float ZrotationDegrees = 0.0f;
		glm::vec3 positionXYZ;


		/*** Render Building											***/
		/*** Set needed transformations before drawing the basic mesh.  ***/
		/*** This same ordering of code should be used for transforming ***/
		/*** and drawing all the basic 3D shapes.						***/
		/******************************************************************/
		// set the XYZ scale for the mesh
		scaleXYZ = glm::vec3(1.0f, 1.0f, 2.0f);

		// set the XYZ rotation for the mesh
		XrotationDegrees = 0.0f + Xrot;
		YrotationDegrees = 0.0f + Yrot;
		ZrotationDegrees = 0.0f + Zrot;

		// set the XYZ position for the mesh
		positionXYZ = glm::vec3(0.0f + Xpos, 0.5f + Ypos, 0.0f + Zpos);

		// set the transformations into memory to be used on the drawn meshes
		SetTransformations(
			scaleXYZ,
			XrotationDegrees,
			YrotationDegrees,
			ZrotationDegrees,
			positionXYZ);

		// set the color for the next draw command
		SetShaderColor(0.91, 0.85, 0.71, 1.0);
		SetShaderTexture("Brick");
		SetTextureUVScale(4.0, 4.0);
		SetShaderMaterial("stone");
		// draw the mesh with transformation values
		m_basicMeshes->DrawBoxMesh();
		/****************************************************************/

		/*** Render Roof												***/
		/*** Set needed transformations before drawing the basic mesh.  ***/
		/*** This same ordering of code should be used for transforming ***/
		/*** and drawing all the basic 3D shapes.						***/
		/******************************************************************/
		// set the XYZ scale for the mesh
		scaleXYZ = glm::vec3(1.0f, 2.05f, 1.0f);

		// set the XYZ rotation for the mesh
		XrotationDegrees = -90.0f + Xrot;
		YrotationDegrees = 0.0f + Yrot;
		ZrotationDegrees = 0.0f + Zrot;

		// set the XYZ position for the mesh
		positionXYZ = glm::vec3(0.0f + Xpos, 1.5f + Ypos, 0.0f + Zpos);

		// set the transformations into memory to be used on the drawn meshes
		SetTransformations(
			scaleXYZ,
			XrotationDegrees,
			YrotationDegrees,
			ZrotationDegrees,
			positionXYZ);

		// set the color for the next draw command
		//SetShaderColor(0.36, 0.16, 0.11, 1.0);
		SetShaderTexture("Roof");
		SetTextureUVScale(1.25, 2.25);
		SetShaderMaterial("roof");
		// draw the mesh with transformation values
		m_basicMeshes->DrawPrismMesh();
		/****************************************************************/

		/*** Render Door												***/
		/*** Set needed transformations before drawing the basic mesh.  ***/
		/*** This same ordering of code should be used for transforming ***/
		/*** and drawing all the basic 3D shapes.						***/
		/******************************************************************/
		// set the XYZ scale for the mesh
		scaleXYZ = glm::vec3(0.25f, 0.50f, 0.25f);

		// set the XYZ rotation for the mesh
		XrotationDegrees = 0.0f + Xrot;
		YrotationDegrees = 90.0f + Yrot;
		ZrotationDegrees = 0.0f + Zrot;

		// set the XYZ position for the mesh
		positionXYZ = glm::vec3(0.39f + Xpos, 0.25f + Ypos, 0.0f + Zpos);

		// set the transformations into memory to be used on the drawn meshes
		SetTransformations(
			scaleXYZ,
			XrotationDegrees,
			YrotationDegrees,
			ZrotationDegrees,
			positionXYZ);


		// set the color for the next draw command
		//SetShaderColor(0.32, 0.10, 0.02, 1.0);
		SetShaderTexture("Wood");
		SetTextureUVScale(1.5, 2.0);
		SetShaderMaterial("wood");
		// draw the mesh with transformation values
		m_basicMeshes->DrawBoxMesh();
		/****************************************************************/

		/*** Render Window												***/
		/*** Set needed transformations before drawing the basic mesh.  ***/
		/*** This same ordering of code should be used for transforming ***/
		/*** and drawing all the basic 3D shapes.						***/
		/******************************************************************/
		// set the XYZ scale for the mesh
		scaleXYZ = glm::vec3(0.375f, 0.375f, 0.25f);

		// set the XYZ rotation for the mesh
		XrotationDegrees = 0.0f + Xrot;
		YrotationDegrees = 90.0f + Yrot;
		ZrotationDegrees = 0.0f + Zrot;

		// set the XYZ position for the mesh
		positionXYZ = glm::vec3(0.39f + Xpos, 0.625f + Ypos, -0.5f + Zpos);

		// set the transformations into memory to be used on the drawn meshes
		SetTransformations(
			scaleXYZ,
			XrotationDegrees,
			YrotationDegrees,
			ZrotationDegrees,
			positionXYZ);

		SetShaderColor(0.41, 0.83, 0.85, 1.0);
		SetShaderMaterial("glass");
		// draw the mesh with transformation values
		m_basicMeshes->DrawBoxMesh();
		/****************************************************************/

			// draw the mesh with transformation values
		m_basicMeshes->DrawBoxMesh();
		/****************************************************************/

		/*** Render Window2												***/
		/*** Set needed transformations before drawing the basic mesh.  ***/
		/*** This same ordering of code should be used for transforming ***/
		/*** and drawing all the basic 3D shapes.						***/
		/******************************************************************/
		// set the XYZ scale for the mesh
		scaleXYZ = glm::vec3(0.375f, 0.375f, 0.25f);

		// set the XYZ rotation for the mesh
		XrotationDegrees = 0.0f + Xrot;
		YrotationDegrees = 90.0f + Yrot;
		ZrotationDegrees = 0.0f + Zrot;

		// set the XYZ position for the mesh
		positionXYZ = glm::vec3(0.39f + Xpos, 0.625f + Ypos, 0.5f + Zpos);

		// set the transformations into memory to be used on the drawn meshes
		SetTransformations(
			scaleXYZ,
			XrotationDegrees,
			YrotationDegrees,
			ZrotationDegrees,
			positionXYZ);

		SetShaderColor(0.41, 0.83, 0.85, 1.0);
		SetShaderMaterial("glass");
		// draw the mesh with transformation values
		m_basicMeshes->DrawBoxMesh();
		/****************************************************************/
	}

/***********************************************************
 *  RenderWindmill()
 *
 *  This method is called to render the shapes for the House object.
 * 
 *  This is the windmill facing left of the camera
 ***********************************************************/
void SceneManager::RenderWindmill()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;


	/*** Render Building											***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.0f, 4.0f, 1.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(12.0f, 0.0f, -4.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color for the next draw command
	SetShaderColor(0.91, 0.85, 0.71, 1.0);
	SetShaderTexture("Brick");
	SetTextureUVScale(4.0, 4.0);
	SetShaderMaterial("stone");
	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	/*** Render Roof											***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.0f, 2.0f, 1.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(12.0f, 4.0f, -4.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color for the next draw command
	//SetShaderColor(0.36, 0.16, 0.11, 1.0);
	SetShaderTexture("Roof");
	SetTextureUVScale(2.0, 2.0);
	SetShaderMaterial("roof");
	// draw the mesh with transformation values
	m_basicMeshes->DrawConeMesh();

	/*** Render Pole											***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.10f, 1.0f, 0.10f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 45.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(11.35f, 3.0f, -3.35f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color for the next draw command
	//SetShaderColor(0.32, 0.10, 0.02, 1.0);
	SetShaderTexture("Wood");
	SetTextureUVScale(1.5, 2.0);
	SetShaderMaterial("wood");
	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	/*** Render Door												***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.25f, 0.50f, 0.25f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 45.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(11.38f, 0.25f, -3.38f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	/*** Render Blade1												***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.05f, 3.0f, 0.35f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 45.0f;
	YrotationDegrees = 45.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(10.75f, 3.0f, -2.75f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	// set the color for the next draw command
	//SetShaderColor(0.32, 0.10, 0.02, 1.0);
	SetShaderTexture("Wood");
	SetTextureUVScale(3.0, 6.0);
	SetShaderMaterial("wood");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/*** Render Blade2												***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.05f, 3.0f, 0.35f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = -45.0f;
	YrotationDegrees = 45.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(10.75f, 3.0f, -2.75f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	// set the color for the next draw command
	//SetShaderColor(0.32, 0.10, 0.02, 1.0);
	SetShaderTexture("Wood");
	SetTextureUVScale(3.0, 6.0);
	SetShaderMaterial("wood");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
}

/***********************************************************
*  RenderWall()
*
*  This method is called to render the shapes for the Wall object.
***********************************************************/
void SceneManager::RenderWall()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Render WallBase											***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(100.0f, 20.0f, 5.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 10.0f, -28.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.6, 0.6, 0.6, 1.0);
	SetShaderMaterial("cement");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/*** Render WallBump											***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(5.0f, 22.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 10.0f, -28.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.6, 0.6, 0.6, 1.0);
	SetShaderMaterial("cement");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/*** Render WallBump											***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(5.0f, 22.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(25.0f, 10.0f, -28.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.6, 0.6, 0.6, 1.0);
	SetShaderMaterial("cement");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/*** Render WallBump											***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(5.0f, 22.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(50.0f, 10.0f, -28.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.6, 0.6, 0.6, 1.0);
	SetShaderMaterial("cement");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/*** Render WallBump											***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(5.0f, 22.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-25.0f, 10.0f, -28.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.6, 0.6, 0.6, 1.0);
	SetShaderMaterial("cement");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/*** Render WallBump											***/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(5.0f, 22.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-50.0f, 10.0f, -28.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.6, 0.6, 0.6, 1.0);
	SetShaderMaterial("cement");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
}