/*************************************************************
Projeto inicial com a PhysX 2.8.4 – Física para Jogos 2011-2
Corpos Rígidos

Código base, iniciado utilizando o exemplo abaixo:
page : http://mmmovania.blogspot.com/2011/04/getting-started-with-nvidia-physx.html

e NxBoxes da documentação da PhysX

Adaptado inicialmente por:
Rossana B. Queiroz -- 23/10/2011

Segunda adaptação por:
Rainer Mansan Vieira -- 03/12/2011

*************************************************************/

// TRABALHO JOGO GRAU B

#include <iostream>
#include <GL/glut.h>

#include <vector>

#include <cstdlib>

#include <ctime>

#include <NxPhysics.h>

//Conjunto de Rotinas para desenho de primitivas, código que vem junto com os exemplos da PhysX
#include "DrawObjects.h"

//Classe para a manipulaçao de malhas
#include "Mesh.h"

//Classe auxiliar para a criação de juntas
#include "Joints.h"

#include "Fort.h"
#include "Enemy.h"

using namespace std;

const int	WINDOW_WIDTH=1024, 
WINDOW_HEIGHT=768;

#define MAX_PATH 512
static int GRID = 150;


//Globais com objetos das principais classes da PhysX
static NxPhysicsSDK* gPhysicsSDK = NULL;

NxScene* gScene = NULL;
NxActor* groundPlane; 
//NxActor* box; 
NxReal myTimestep = 1.0f/60.0f;

NxActor* theBox;

//Globais para navegação com o Mouse
int oldX=0, oldY=0;
int absx = 0;
int absy = 0;
float rX=15, rY=0;
float fps=0;
int startTime=0;
int totalFrames=0;
int state =1 ;


float force = 0;

float xpos = 0;
float zpos = 0;
float ypos = 0;

enum Mat {DEFAULT = 0, MADEIRA = 1, FERRO = 2, PEDRA = 3, BORRACHA = 4};

Mat currentMaterial = MADEIRA;

Fort* theFort = NULL;

vector<Enemy> enemies;
vector<NxActor*> _enemies;
vector <NxVec3> spawners;

int spawnRate = 50;
float enemySpeed = 20;
int startTimer = 600;
int timer = 0;

int points = 0;
int damage = 0;
int ammo = 10;

bool hold = false;

void treatCollision(NxContactPair& pair);

class MyContactReport : public NxUserContactReport    
 {        
	 void onContactNotify(NxContactPair& pair, NxU32 events)        
	 {
		 if ((pair.isDeletedActor[0]) || (pair.isDeletedActor[1]))
			 return;
		 
		treatCollision(pair);

		 //You can read the contact information out of the        
		 //contact pair data here.        
	 }    
 } myReport;


//Alguns cabeçalhos de funções -- Essas funções são funções exemplo de criação de cada tipo de 
//Corpo rígido que a PhysX suporta. Você deve criar funções que generalizem os principais parâmetros
//Para que possa criar objetos diferenciados na cena
NxActor *CreateSphere(NxVec3 pos, float radius, float mass, int material);
//NxActor*  CreateSphere2(NxVec3 pos, float radius, float dens);
	NxActor* CreateStaticSphere(NxVec3 pos, float radius);
NxActor* CreateBox(NxVec3 pos, NxVec3 dim, float dens);
	NxActor* CreateStaticBox(NxVec3 pos, NxVec3 dim);
NxActor* CreateCapsule(const NxVec3& pos, const NxReal height, const NxReal radius, const NxReal density);
NxActor* CreatePrimaryMultiShape();
NxActor* CreateConvexMesh(NxVec3 pos, NxVec3 siz, float dens);
NxActor* CreateConvexMesh2(NxVec3 pos, NxVec3 siz, float dens);
NxActor* CreateTriangleMesh(NxVec3 pos, NxVec3 siz, float dens);

NxPulleyJoint* pulleyJoint(NxActor *obj1, NxActor *obj2, NxVec3 pulley1, NxVec3 pulley2, NxReal dist, NxReal ratio);
NxRevoluteJoint* motor(NxActor *actor1, NxActor* actor2, NxReal maxVel, NxVec3 anchor, NxVec3 axis);

void RotateObject(NxActor* actor, NxVec3 eixo, NxReal angle);
NxVec3 ApplyForceToActor(NxActor* actor, const NxVec3& forceDir, const NxReal forceStrength);

//Algumas flags
bool isPaused = false;
bool drawConvex = true;

//Um vetor com as estruturas de mesh
std::vector <Mesh> meshes;


//Definição da câmera sintética
void SetOrthoForFont()
{	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
	glScalef(1, -1, 1);
	glTranslatef(0, -WINDOW_HEIGHT, 0);
	glMatrixMode(GL_MODELVIEW);	
	glLoadIdentity();
}

//Resetando a câmera
void ResetPerspectiveProjection() 
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}


//Para escrever texto na tela
void RenderSpacedBitmapString( int x, int y, int spacing, void *font, char *string) 
{
	char *c;
	int x1=x;
	for (c=string; *c != '\0'; c++) {
		glRasterPos2i(x1,y);
		glutBitmapCharacter(font, *c);
		x1 = x1 + glutBitmapWidth(font,*c) + spacing;
	}
}


//Para desenhar aquela grid em cima do plano de chão
void DrawGrid(int grid)
{
	glBegin(GL_LINES);
	glColor3f(0.f, 0.f, 0.f);
	for (int i = -grid; i <= grid; i++)
	{
		glVertex3f((float)i,0,(float)-grid);
		glVertex3f((float)i,0,(float)grid);

		glVertex3f((float)-grid,0,(float)i);
		glVertex3f((float)grid,0,(float)i);
	}
	glEnd();
}

//Para desenhar os modelos geométricos das meshes
void RenderMesh(Mesh m){ 

	float glmat[16];

	glEnable(GL_TEXTURE_2D);
   
	if (!m.actors.size() ) {
		glPushMatrix();
		m.actor->getGlobalPose().getColumnMajor44(glmat);
		glMultMatrixf(glmat);
	
	    // Para visualizar em 'wireframe', descomente esta linha
		//SetaModoDesenho('w');
		//Rotina da bibutil para desenhar objetos *OBJ
		DesenhaObjeto(m.obj);
		//E se estiver em wireframe, e quiseres desenhar o centro de massa, descomente essa
		//DrawPoint(m.actor->getCMassLocalPosition());
		glPopMatrix();
	}
	else {
		for(int i=0; i<m.actors.size(); i++){
			if(m.actors[i]){
				glPushMatrix();
				m.actors[i]->getGlobalPose().getColumnMajor44(glmat);
				glMultMatrixf(glmat);
	
				// Para visualizar em 'wireframe', descomente esta linha
				//SetaModoDesenho('w');
				DesenhaMesh(m.obj->meshes[i]);
				glPopMatrix();
			}
		}
	}
	
	glDisable(GL_TEXTURE_2D);
	
}

void treatCollision(NxContactPair& pair)
{
	
		NxActor* actor1 = pair.actors[0];
		NxActor* actor2 = pair.actors[1];

	// SE UM INIMIGO COLIDE COM O FORTE
	if ((actor1->getGroup() == 1) || (actor2->getGroup() == 1))
	{
		damage += 10;
		points--;

		if (actor1->getGroup() == 2)
		{
			gScene->releaseActor(*actor1);
		}
		else if (actor2->getGroup() == 2)
		{
			gScene->releaseActor(*actor2);
		}
		return;
	}

	// SE UM TIRO COLIDE...
	if ((actor1->getGroup() == 3) || (actor2->getGroup() == 3))
	{
		//	 COM UM INIMIGO
		if ((actor1->getGroup() == 2) || (actor2->getGroup() == 2))
		{
			gScene->releaseActor(*actor1);
			gScene->releaseActor(*actor2);
			actor1 = NULL;
			actor2 = NULL;
			points++;
			ammo += 5;
			return;
		}

		
	}
		
}

void createFort()
{
	theFort = new Fort(NxVec3(0,0,0),NxVec3(5,3,5), 100);
	theFort->actor = CreateStaticBox(theFort->position, theFort->size);
	//RotateObject(theFort.actor, NxVec3(0,1,0), 45);

	theFort->actor->setGroup(1);

	/*NxBoxShapeDesc boxDesc;
	boxDesc.dimensions = theFort->size * 1,2;
	boxDesc.shapeFlags|=NX_TRIGGER_ENABLE;
	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&boxDesc);
	theFort->trigger = gScene->createActor(actorDesc);*/
}

void createEnemy(NxVec3 pos, float size, float spd, float dam)
{
	/*Enemy *ptrenemy = new Enemy(spd, dam, -pos);
	ptrenemy->actor = CreateSphere(pos, size, 5.0,	size);
	ApplyForceToActor(ptrenemy->actor, ptrenemy->target, ptrenemy->speed);
	ptrenemy->actor->setGroup(2);
	
	enemies.push_back(*ptrenemy);*/

	//Enemy *ptrenemy = new Enemy(spd, dam, -pos);
	NxActor* new_enemy = NULL;
	//new_enemy = CreateSphere(NxVec3(0,5,0), 0.1, 5.0, 2);
	new_enemy = CreateSphere(pos, size, 5.0, 2);
	new_enemy->setGroup(2);
	ApplyForceToActor(new_enemy, -pos, spd);
	
	
	

}


void pushCreatures()
{
	for (int i = 0; i < enemies.size(); i++)
	{
		Enemy* en = &enemies[i];
		ApplyForceToActor(en->actor, en->target, en->speed);
	}
}

void setSpawners()
{
	int d = 100;
	int a = 0;

	spawners.push_back(NxVec3(d,a,d));
	spawners.push_back(NxVec3(d,a,0));
	spawners.push_back(NxVec3(d,a,-d));

	spawners.push_back(NxVec3(0,a,d));
	spawners.push_back(NxVec3(0,a,-d));
	
	spawners.push_back(NxVec3(-d,a,d));
	spawners.push_back(NxVec3(-d,a,0));
	spawners.push_back(NxVec3(-d,a,-d));
	
}

void SpawnCreatures()
{
	
		
		for (int i = 0; i < spawners.size(); i++)
		{
			
			if (rand() % 100 <= spawnRate)
				createEnemy(spawners[i], rand() % 3 + 1, enemySpeed, 2);
		}

		spawnRate += 1;
		enemySpeed += 0.5;
	
	
}


void setGame()
{
	
	srand(time(0));

	// create fort
	createFort();

	//SpawnCreatures();

	// set spawners
	setSpawners();

	// colisoes
	// 1 = forte
	// 2 = inimigos
	// 3 = tiros
	// forte e inimigos
	gScene->setActorGroupPairFlags(1,2, NX_NOTIFY_ON_START_TOUCH);

	// tiros e inimigos
	gScene->setActorGroupPairFlags(3,2, NX_NOTIFY_ON_START_TOUCH);
	

	// posicao inicial do jogador
	xpos = 0;
	zpos = 0;
	ypos = 1.8 + theFort->size.y;
	
}

void stepGame()
{
	if (timer <= 0)
	{
		SpawnCreatures();
		pushCreatures();
		timer = startTimer;
		
		ammo+= 5;
	}
	else
		timer--;
	

	if (hold == true)
	{
		force += 50;
		if (force >=3000)
			force = 3000;
	}

	
}

//Para chamar o passo da simulação da PhysX
void StepPhysX() 
{ 
	if (!isPaused) {
		stepGame();

		gScene->simulate(myTimestep);        
		gScene->flushStream();     

		

		//...perform useful work here using previous frame's state data        
		while(!gScene->fetchResults(NX_RIGID_BODY_FINISHED, false) )     
		{
			
			// do something useful        
		}
		
	}	
} 

void setMaterials()
{
	//Define o material padrão
	NxMaterial* defaultMaterial = gScene->getMaterialFromIndex(0);
	defaultMaterial->setRestitution(0.1);
	defaultMaterial->setStaticFriction(0.1);
	defaultMaterial->setDynamicFriction(0.1);

	// MADEIRA
	
	NxMaterial* wood = gScene->getMaterialFromIndex(1);
	wood->setRestitution(0.5);
	wood->setStaticFriction(0.6);
	wood->setDynamicFriction(0.6);
	//createMaterial (wood)

	// FERRO
	NxMaterial* iron = gScene->getMaterialFromIndex(2);
	iron->setRestitution(0.5);
	iron->setStaticFriction(0.8);
	iron->setDynamicFriction(0.8);

	// PEDRA
	NxMaterial* stone = gScene->getMaterialFromIndex(3);
	stone->setRestitution(0.1);
	stone->setStaticFriction(1);
	stone->setDynamicFriction(1);

	// BORRACHA
	NxMaterial* rubber = gScene->getMaterialFromIndex(4);
	rubber->setRestitution(0.9);
	rubber->setStaticFriction(0.5);
	rubber->setDynamicFriction(0.5);
}

//Rotina que faz a inicialização da simulação física
void InitializePhysX() {
	
	//Define a instância da SDK
	gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);
	if(gPhysicsSDK == NULL) {
		cerr<<"Error creating PhysX device."<<endl;
		cerr<<"Exiting..."<<endl;
		exit(1);
	}

	//Cria a cena
	NxSceneDesc sceneDesc;
	sceneDesc.gravity.set(0.0f, -9.8f, 0.0f);
	sceneDesc.simType = NX_SIMULATION_SW;
	gScene = gPhysicsSDK->createScene(sceneDesc);

	//Seta parâmetros da simulação: timestep, max_iterations e método de integração
	gScene->setTiming(myTimestep / 4.0f, 4, NX_TIMESTEP_FIXED);

	
	setMaterials();
	//Criar material madeira...

	//Cria o plano de chão
	NxPlaneShapeDesc planeDesc; //Descritor de forma de plano
	NxActorDesc actorDesc; //Descritor de corpo rígido: ator
	actorDesc.shapes.pushBack(&planeDesc); //adiciona forma ao ator
	NxActor* deplane = gScene->createActor(actorDesc); //cria o ator

	//RotateObject(deplane,NxVec3(1,0,0),45);
	
	gScene->setUserContactReport(&myReport);

	setGame();

}


//Rotina que percorre os atores (corpos rígidos) da cena e manda desenhá-los na tela
void RenderActors() 
{ 
    // Render all the actors in the scene 
    int nbActors = gScene->getNbActors(); 
    NxActor** actors = gScene->getActors(); 
    while (nbActors--) 
    { 
		glColor3f(1,1,1);	
        NxActor* actor = *actors++; 
		
		/*if (!actor->getShapes()[0]->isConvexMesh() && !actor->getShapes()[0]->isTriangleMesh())*/
			DrawActor(actor);
		/*else if (drawConvex)
			 DrawWireActor(actor);*/

		
		DrawActorShadow(actor);
        
		if (actor->getShapes()[0]->isPlane())
		{
			//Draw the grid and axes
			glDisable(GL_LIGHTING);
			/*DrawAxes();	*/
			DrawGrid(GRID);
			glEnable(GL_LIGHTING);
		}
		
    } 


} 

//Rotina para finalizar a simulação 
void ShutdownPhysX() {
	gPhysicsSDK->releaseScene(*gScene);
	NxReleasePhysicsSDK(gPhysicsSDK);
}

//Rotina que converte graus para radianos
float GrausParaRadianos(float angulo)
{
	return angulo*NxPi/180;
}

//Rotina que converte radianos para graus
float RadianosParaGraus(float angle){
    return angle*180/NxPi;
}

NxVec3 CalculaComponentesDoVetor(float magnitude, float fi, float theta)
{
	NxVec3 vec;
	vec.x = magnitude * sin(fi) * cos(theta) ;
	vec.z = magnitude * sin(fi) * sin(theta) ;
	vec.y = magnitude * cos(fi);
	return vec;
}

bool outsideFortLimits(float x, float z)
{
	if (theFort != NULL)
	{
		float xmax = theFort->position.x + (theFort->size.x);
		float xmin = theFort->position.x - (theFort->size.x);
		float zmax = theFort->position.z + (theFort->size.z);
		float zmin = theFort->position.z - (theFort->size.z);
		if ((x >= xmax) || (x <= xmin) || (z >= zmax) || (z <= zmin))
			return true; 
	}
	return false;
}


//Rotina de callback de teclado
void KeyboardCallback(unsigned char key, int x, int y)
{

	switch (key)
	{
		case 'w':
			{
			if (!isPaused)
				{
				float xrotrad, yrotrad;
				yrotrad = (rY / 180 * 3.141592654f);
				xrotrad = (rX / 180 * 3.141592654f);
				if (!outsideFortLimits(xpos + float(sin(yrotrad)) *0.3, zpos - float(cos(yrotrad)) *0.3))
					{
						xpos += float(sin(yrotrad)) *0.3;
						zpos -= float(cos(yrotrad)) *0.3;
					}
				//ypos -= float(sin(xrotrad)) ;
				}
			break;
			}

		case 's':
			{
			if (!isPaused)
				{
					float xrotrad, yrotrad;
					yrotrad = (rY / 180 * 3.141592654f);
					xrotrad = (rX / 180 * 3.141592654f);

					if (!outsideFortLimits(xpos - float(sin(yrotrad)) *0.2, zpos + float(cos(yrotrad)) *0.2))
					{
					xpos -= float(sin(yrotrad)) *0.2;
					zpos += float(cos(yrotrad)) *0.2;
					}
					//ypos += float(sin(xrotrad));
				
				}
			break;
			}

		case 'd':
			{
			if (!isPaused)
				{
				float yrotrad;
				yrotrad = (rY / 180 * 3.141592654f);

				if (!outsideFortLimits(xpos + float(sin(yrotrad)) *0.2, zpos + float(cos(yrotrad)) *0.2))
				{
				xpos += float(cos(yrotrad)) * 0.2;
				zpos += float(sin(yrotrad)) * 0.2;
				}
				
				}
			break;
			}


		case 'a':
			{
			if (!isPaused)
				{
					float yrotrad;
					yrotrad = (rY / 180 * 3.141592654f);

					if (!outsideFortLimits(xpos - float(sin(yrotrad)) *0.2, zpos - float(cos(yrotrad)) *0.2))
					{
					xpos -= float(cos(yrotrad)) * 0.2;
					zpos -= float(sin(yrotrad)) * 0.2;
					}
					
				}
			break;
			}
		case 'z':
			{
				currentMaterial = MADEIRA;
				break;
			}
		case 'x':
			{
				currentMaterial = FERRO;
				break;
			}
		case 'c':
			{
				currentMaterial = PEDRA;
				break;
			}
		case 'v':
			{
				currentMaterial = BORRACHA;
				break;
			}
		case ' ':
		{
			
			break;
		}

		case 'p':
			isPaused = !isPaused;
			break;
		case 27:
			{
				//ShutdownPhysX(); //tá dando erro ao liberar, ver isso mais tarde
				exit(0);
			}
	}
}

//Rotina que aplica uma força especificada em um ator
NxVec3 ApplyForceToActor(NxActor* actor, const NxVec3& forceDir, const NxReal forceStrength)
{
	NxVec3 forceVec = forceStrength*forceDir;
	
	if (actor != NULL)
	{
	actor->addForce(forceVec); //aplica a força no centro de massa 
	//gForcePos = actor->getCMassGlobalPosition(); //para desenho depois
	}
	return forceVec;
}


//Rotina para a inicialização dos parâmetros da OpenGL
void InitGL() { 


	//Define o modo de operação da GLUT
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	 glEnable(GL_DEPTH_TEST);
     glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	float AmbientColor[]    = { 0.0f, 0.1f, 0.5f, 0.0f };         glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
    float DiffuseColor[]    = { 0.7f, 0.7f, 0.7f, 0.0f };         glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
    float SpecularColor[]   = { 0.5f, 0.5f, 0.5f, 0.0f };         glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);

	glDisable(GL_LIGHTING);

	glutWarpPointer(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
	glutSetCursor(GLUT_CURSOR_NONE); 
}

//Rotina de callback para redimensionamento da tela
void OnReshape(int nw, int nh) {
	glViewport(0,0,nw, nh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat)nw / (GLfloat)nh, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}



char buffer[MAX_PATH];
char buffer2[MAX_PATH];
char buffer3[MAX_PATH];
char buffer4[MAX_PATH];

////////////////////////////////////// RENDER /////////////////////////////////////////////

void OnRender() {

	// Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Calculate fps
	totalFrames++;
	int current = glutGet(GLUT_ELAPSED_TIME);
	if((current-startTime)>1000)
	{		
		float elapsedTime = float(current-startTime);
		fps = ((totalFrames * 1000.0f)/ elapsedTime) ;
		startTime = current;
		totalFrames=0;
	}

	sprintf_s(buffer, "Points : %i ",points);
	sprintf_s(buffer2, "Ammo : %i ", ammo);
	sprintf_s(buffer3, "Damage : %i ",damage);
	//sprintf_s(buffer4, " ",points);

	//Update PhysX	
    if (gScene) 
    { 
        StepPhysX();
    } 

	glLoadIdentity();
	//camera();
	
	glRotatef(rX,1,0,0);
	glRotatef(rY,0,1,0);
	glTranslatef(-xpos,-ypos,-zpos);

	glEnable(GL_LIGHTING);
	
	//Para desenhar os atores da cena
	RenderActors();

	//Para renderizar a geometria associada as meshes
	for(int i=0;i<meshes.size();i++){
		RenderMesh(meshes[i]);
	}
	

	
	glDisable(GL_LIGHTING);
	SetOrthoForFont();		
	glColor3f(1,1,1);
	//Show the fps
	RenderSpacedBitmapString(10, WINDOW_HEIGHT * 0.05,0,GLUT_BITMAP_HELVETICA_18,buffer);
	RenderSpacedBitmapString(WINDOW_WIDTH *0.9,20,0,GLUT_BITMAP_HELVETICA_18,buffer2);
	RenderSpacedBitmapString(WINDOW_WIDTH *0.9,50,0,GLUT_BITMAP_HELVETICA_18,buffer3);
	//RenderSpacedBitmapString(20,20,0,GLUT_BITMAP_HELVETICA_12,buffer4);
	
	

	RenderSpacedBitmapString(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2,0,GLUT_BITMAP_HELVETICA_18,"-|-");
	
	//Se está pausado, avisa
	if (isPaused)
		RenderSpacedBitmapString(20,40,0,GLUT_BITMAP_HELVETICA_18,"PAUSADO! PRESSIONE 'p' PARA CONTINUAR A SIMULACAO!");

	ResetPerspectiveProjection();

	glutSwapBuffers();

	

	
}

//Rotina chamada para finalizar a aplicação
void OnShutdown() {
	ShutdownPhysX();
}

void ShootBall()
{

				float yrotrad = (rY / 180 * 3.141592654f);
				float xrotrad = (rX / 180 * 3.141592654f);
				float vecx = float(sin(yrotrad)) ;
				float vecz = -1 * float(cos(yrotrad)) ;
				float vecy = -1 * float(sin(xrotrad)) ;
				
				NxVec3 posi(xpos,ypos,zpos);
				NxVec3 vec(vecx ,vecy,vecz);
				//calculando as componentes
				
				NxActor *sphere;
				
				switch (currentMaterial)
				{
					case MADEIRA:
						sphere = CreateSphere(posi,0.1,0.5,1);
						break;
					case FERRO:
						sphere = CreateSphere(posi,0.1,0.5,2);
						break;
					case PEDRA:
					sphere = CreateSphere(posi,0.1,0.5,3);
						break;
					case BORRACHA:
					sphere = CreateSphere(posi,0.1,0.5,4);
						break;

				}

				sphere->setGroup(3);
							
				NxVec3 forceVec;
				//forceVec.multiply(3000,vec); //1000 é a magnitude, estou multiplicando pela direção
				forceVec.multiply(force,vec);
				//Adicionando a força ao centro de massa da esfera criada
				if(sphere)
					sphere->addForce(forceVec); 

				force = 0;
				hold = false;

}


//Rotina de callback de mouse
void Mouse(int button, int s, int x, int y)
{
	
	if (s == GLUT_DOWN)
	{
		hold = true;
	}
	if (s == GLUT_UP) 
	{
		if ((!isPaused) && (ammo > 0))
		{

		ShootBall();
		ammo--;
		}
	}
}

void Motion(int x, int y)
{
	
		absx = x - oldX;
		absy = y - oldY ;

		rY += absx * 0.8f;
		rX += absy * 0.8f;
		
		oldX = x; 
		oldY = y;

		// limita o angulo em Y
		if (rX > 400) 
			rX = 400;
		else if (rX < 300)
			rX = 300;

		glutPostRedisplay(); 
	

}

void MouseMove(int x, int y)
{
	
		absx = x - oldX;
		absy = y - oldY ;

		rY += absx * 0.8f;
		rX += absy * 0.8f;
		
		oldX = x; 
		oldY = y;

		// limita o angulo em Y
		if (rX > 400) 
			rX = 400;
		else if (rX < 300)
			rX = 300;

		glutPostRedisplay(); 
	

}

//Rotina de callback Idle
void OnIdle() {
	glutPostRedisplay();
}

//MAIN////////////////////////////// MAIN/////////////////////////////////// MAIN //////////

//Rotina principal :)
void main(int argc, char** argv) {
	atexit(OnShutdown);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Iniciando com a PhysX");

	glutDisplayFunc(OnRender);
	glutIdleFunc(OnIdle);
	glutReshapeFunc(OnReshape);

	glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
	glutPassiveMotionFunc(MouseMove);
	//glutWarpPointer(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

	glutKeyboardFunc(KeyboardCallback);
	/*glutSpecialFunc(SpecialCallback);
	glutSpecialUpFunc(SpecialUpCallback);*/

	InitGL();

	InitializePhysX();

	glutMainLoop();		
}


//Rotinas de Criação de Corpos Rígidos não parametrizados, para exemplo de aula

NxPulleyJoint* pulleyJoint(NxActor *obj1, NxActor *obj2, NxVec3 pulley1, NxVec3 pulley2, NxReal dist, NxReal ratio)
{
	NxMotorDesc gMotorDesc;
	gMotorDesc.maxForce = NX_MAX_REAL;		
    gMotorDesc.freeSpin = false;
    gMotorDesc.velTarget = 0;

	NxVec3 globalAxis = NxVec3(0,-1,0);

	CreateStaticSphere(pulley1, 0.5f);
	CreateStaticSphere(pulley2, 0.5f);
	//ball->
	return CreatePulleyJoint(obj1, obj2, pulley1, pulley2, globalAxis, dist, ratio, gMotorDesc);
}

NxRevoluteJoint* motor(NxActor *actor1, NxActor* actor2, NxReal maxVel, NxVec3 anchor, NxVec3 axis)
{
	// Para testar as juntas com um MOTOR
	//NxActor *box1 = CreateBox(NxVec3(0,5,0), NxVec3(0.5,2,1), 10);
    actor1->raiseBodyFlag(NX_BF_KINEMATIC);
	//actor1->bod
    //NxActor *box2 = CreateBox(NxVec3(0,1,0), NxVec3(0.5,2,1), 10);
    actor2->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);

    actor2->setAngularDamping(0);
    actor2->setMaxAngularVelocity(maxVel);

    return CreateRevoluteJointWithAMotor(actor1, actor2, anchor, axis);
    //gSelectedActor = box2;
}

//Cria uma esfera
//Rotina para a criação de uma esfera parametrizada (ainda usa material padrão)

NxActor *CreateSphere(NxVec3 pos, float radius, float mass, int material){

	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a box, 1m on a side
	NxSphereShapeDesc sphereDesc;
	sphereDesc.radius		= radius;
	sphereDesc.localPose.t	= NxVec3(0, 0, 0); //posicao local, deixar em 0,0,0
	sphereDesc.materialIndex = material; //default  //createMaterial(0.5,0.5,0.5);
	
	actorDesc.shapes.pushBack(&sphereDesc);

	actorDesc.body			= &bodyDesc;
	actorDesc.density		= NxComputeSphereDensity(sphereDesc.radius,mass);
	actorDesc.globalPose.t	= pos;	
	assert(actorDesc.isValid());  

	NxActor *pActor = gScene->createActor(actorDesc);	

    //pActor->setGlobalOrientationQuat(ori);

	return pActor;
}


NxActor* CreateStaticSphere(NxVec3 pos, float radius)
{
	// Set the sphere starting height to 3.5m so box starts off falling onto the ground
	//NxReal sphereStartHeight = 3.5; 

	// Add a single-shape actor to the scene
	
	

	// The actor has one shape, a sphere, 1m on radius
	NxSphereShapeDesc sphereDesc;
	sphereDesc.radius		= radius;;
	sphereDesc.localPose.t	= NxVec3(0, 0, 0);

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&sphereDesc);
	actorDesc.globalPose.t	= pos;

	//NxBodyDesc bodyDescriptor;
	actorDesc.body			= NULL;
	
	//actorDesc.raiseBodyFlag(NX_BF_KINEMATIC);

	return gScene->createActor(actorDesc);
}

NxActor* CreateCapsule(const NxVec3& pos, const NxReal height, const NxReal radius, const NxReal density)
{
	assert(0 != gScene);

	NxCapsuleShapeDesc capsuleDesc;
	capsuleDesc.height = height;
	capsuleDesc.radius = radius;
	capsuleDesc.localPose.t = NxVec3(0, radius + 0.5f * height, 0);

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&capsuleDesc);
	actorDesc.globalPose.t = pos;

	NxBodyDesc bodyDesc;
	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = 0;
	}
	return gScene->createActor(actorDesc);	
}

NxActor* CreatePrimaryMultiShape()
{	
	// Create an actor which is composed of Box, Capsule and Sphere.
	NxActorDesc actorDesc;
	NxBodyDesc  bodyDesc;	
	
	//Box Shape
	NxBoxShapeDesc boxShape;
	boxShape.dimensions		= NxVec3(1.0f, 1.0f, 1.0f);
	boxShape.localPose.t	= NxVec3(1.5f, 0.0f, 0.0f);
	assert(boxShape.isValid());
	actorDesc.shapes.pushBack(&boxShape);

	//Capsule Shape
	NxCapsuleShapeDesc capsuleDesc;
	capsuleDesc.radius		= 0.8f;
	capsuleDesc.height		= 1.0f;
	capsuleDesc.localPose.t = NxVec3(0.0f, 0.0f, 0.0f);
	//Rotate capsule 90 degree around z axis
	NxQuat quat(90.0f, NxVec3(0, 0, 1));
	NxMat33 m;
	m.id();
	m.fromQuat(quat);
	capsuleDesc.localPose.M = m;
	assert(capsuleDesc.isValid());
	actorDesc.shapes.pushBack(&capsuleDesc);

	//Sphere Shape
	NxSphereShapeDesc sphereDesc;
	sphereDesc.radius		= 1.0f;
	sphereDesc.localPose.t	= NxVec3(-1.5f, 0.0f, 0.0f);
	assert(sphereDesc.isValid());
	actorDesc.shapes.pushBack(&sphereDesc);

	actorDesc.body			= &bodyDesc;
	actorDesc.density		= 1.0f;
	actorDesc.globalPose.t	= NxVec3(5.0f, 0.0f, 0.0f);
	assert(actorDesc.isValid());
	NxActor *pActor = gScene->createActor(actorDesc);
	assert(pActor);
return pActor;
}

NxActor* CreateStaticBox(NxVec3 pos, NxVec3 dim)
{
	
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	 
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions.set(dim.x,dim.y,dim.z); 
	boxDesc.localPose.t = NxVec3(0, 0, 0); //posição local (em relação à caixa)
	//Adiciona a forma ao ator
	actorDesc.shapes.pushBack(&boxDesc);
	//O ator é um corpo rígido, possui um corpo, uma densidade* e sua posição  no mundo
	actorDesc.body	= NULL;
	//actorDesc.density	= dens;
	NxVec3 posicao;
	
	actorDesc.globalPose.t	= pos;	
	assert(actorDesc.isValid());
 
	//Cria o ator em função do seu descritor
	NxActor *pActor = gScene->createActor(actorDesc);	
 
	// Para iniciar a caixa rotacionada em 45 graus em Y
 //   NxVec3 v(0,1,0); //eixo de rotação
 //   NxReal ang = 45; //ângulo
 //   NxQuat q;
 //   q.fromAngleAxis(ang, v); //para transformar em um quaternion
	//pActor->setGlobalOrientationQuat(q);
 
	return pActor;
}

NxActor* CreateBox(NxVec3 pos, NxVec3 dim, float dens)
{
	
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	 
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions.set(dim.x,dim.y,dim.z); 
	boxDesc.localPose.t = NxVec3(0, 0, 0); //posição local (em relação à caixa)
	//Adiciona a forma ao ator
	actorDesc.shapes.pushBack(&boxDesc);
	//O ator é um corpo rígido, possui um corpo, uma densidade* e sua posição  no mundo
	actorDesc.body	= &bodyDesc;
	actorDesc.density	= dens;
	NxVec3 posicao;
	
	actorDesc.globalPose.t	= pos;	
	assert(actorDesc.isValid());
 
	//Cria o ator em função do seu descritor
	NxActor *pActor = gScene->createActor(actorDesc);	
 
	// Para iniciar a caixa rotacionada em 45 graus em Y
 //   NxVec3 v(0,1,0); //eixo de rotação
 //   NxReal ang = 45; //ângulo
 //   NxQuat q;
 //   q.fromAngleAxis(ang, v); //para transformar em um quaternion
	//pActor->setGlobalOrientationQuat(q);
 
	return pActor;
}

void RotateObject(NxActor* actor, NxVec3 eixo, NxReal angle)
{
	NxQuat quaternion;
	quaternion.fromAngleAxis(angle,eixo);

	actor->setGlobalOrientationQuat(quaternion);
	
}

NxActor* CreateConvexMesh(NxVec3 pos, NxVec3 siz, float dens)
{
    NxActorDesc actorDesc;
    NxBodyDesc bodyDesc;
	
    NxVec3 boxDim(siz.x,siz.y,siz.z);
    // Pyramid
     NxVec3 verts[8] =	
		{	NxVec3(boxDim.x,      -boxDim.y,  -boxDim.z), 
		NxVec3(-boxDim.x,     -boxDim.y,  -boxDim.z), 
		NxVec3(-boxDim.x,     -boxDim.y,  boxDim.z),
		NxVec3(boxDim.x,      -boxDim.y,  boxDim.z), 
		NxVec3(boxDim.x*0.5,   boxDim.y,  -boxDim.z*0.5), 
		NxVec3(-boxDim.x*0.5,  boxDim.y,  -boxDim.z*0.5),
		NxVec3(-boxDim.x*0.5,  boxDim.y,  boxDim.z*0.5), 
		NxVec3(boxDim.x*0.5,   boxDim.y,  boxDim.z*0.5)
	};

	for (int i = 0; i < 8; i++)
	{
		verts[i].x += pos.x;
		verts[i].y += pos.y;
		verts[i].z += pos.z;
	}


	NxConvexMeshDesc *convexDesc = NULL;
	// Create descriptor for convex mesh
	if (!convexDesc)
	{
		convexDesc	= new NxConvexMeshDesc();
		assert(convexDesc);
	}
    	convexDesc->numVertices			= 8;
    	convexDesc->pointStrideBytes		= sizeof(NxVec3);
    	convexDesc->points				= verts;
	convexDesc->flags				= NX_CF_COMPUTE_CONVEX;

	NxConvexShapeDesc convexShapeDesc;
	convexShapeDesc.localPose.t		= NxVec3(0,boxDim.y,0);
	convexShapeDesc.userData		= convexDesc;
 
	
	// Two ways on cooking mesh: 1. Saved in memory, 2. Saved in file	
	NxInitCooking();
	// Cooking from memory
    	MemoryWriteBuffer buf;
    	bool status = NxCookConvexMesh(*convexDesc, buf);
	//
	// Please note about the created Convex Mesh, user needs to release it when no //one uses it to save memory. It can be detected
	// by API "meshData->getReferenceCount() == 0". And, the release API is //"gPhysicsSDK->releaseConvexMesh(*convexShapeDesc.meshData);"

	NxConvexMesh *pMesh = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	assert(pMesh);
    	convexShapeDesc.meshData	= pMesh;
	NxCloseCooking();

    if (pMesh)
    {
      // Save mesh in userData for drawing.
	pMesh->saveToDesc(*convexDesc);
	//
	NxActorDesc actorDesc;
	assert(convexShapeDesc.isValid());
      actorDesc.shapes.pushBack(&convexShapeDesc);
	actorDesc.body		= &bodyDesc;
	actorDesc.density		= 1.0f;
	   
      actorDesc.globalPose.t  = NxVec3(0.0f, 0.0f, 0.0f);
	assert(actorDesc.isValid());
	NxActor* actor = gScene->createActor(actorDesc);
	assert(actor);	
	return actor;
    }

    return NULL;
}

NxActor* CreateConvexMesh2(NxVec3 pos, NxVec3 siz, float dens)
{
    NxActorDesc actorDesc;
    NxBodyDesc bodyDesc;
	
    NxVec3 boxDim(siz.x,siz.y,siz.z);
    // Pyramid
     NxVec3 verts[8] =	
		{	NxVec3(boxDim.x,      -boxDim.y,  -boxDim.z), 
		NxVec3(-boxDim.x,     -boxDim.y,  -boxDim.z), 
		NxVec3(-boxDim.x,     -boxDim.y,  boxDim.z),
		NxVec3(boxDim.x,      -boxDim.y,  boxDim.z), 
		NxVec3(boxDim.x*0.5,   boxDim.y,  -boxDim.z*0.5), 
		NxVec3(boxDim.x*0.8,  boxDim.y,  -boxDim.z*0.5),
		NxVec3(boxDim.x*0.8,  boxDim.y,  boxDim.z*0.5), 
		NxVec3(boxDim.x*0.5,   boxDim.y,  boxDim.z*0.5)
	};

	for (int i = 0; i < 8; i++)
	{
		verts[i].x += pos.x;
		verts[i].y += pos.y;
		verts[i].z += pos.z;
	}


	NxConvexMeshDesc *convexDesc = NULL;
	// Create descriptor for convex mesh
	if (!convexDesc)
	{
		convexDesc	= new NxConvexMeshDesc();
		assert(convexDesc);
	}
    	convexDesc->numVertices			= 8;
    	convexDesc->pointStrideBytes		= sizeof(NxVec3);
    	convexDesc->points				= verts;
	convexDesc->flags				= NX_CF_COMPUTE_CONVEX;

	NxConvexShapeDesc convexShapeDesc;
	convexShapeDesc.localPose.t		= NxVec3(0,boxDim.y,0);
	convexShapeDesc.userData		= convexDesc;
 
	
	// Two ways on cooking mesh: 1. Saved in memory, 2. Saved in file	
	NxInitCooking();
	// Cooking from memory
    	MemoryWriteBuffer buf;
    	bool status = NxCookConvexMesh(*convexDesc, buf);
	//
	// Please note about the created Convex Mesh, user needs to release it when no //one uses it to save memory. It can be detected
	// by API "meshData->getReferenceCount() == 0". And, the release API is //"gPhysicsSDK->releaseConvexMesh(*convexShapeDesc.meshData);"

	NxConvexMesh *pMesh = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	assert(pMesh);
    	convexShapeDesc.meshData	= pMesh;
	NxCloseCooking();

    if (pMesh)
    {
      // Save mesh in userData for drawing.
	pMesh->saveToDesc(*convexDesc);
	//
	NxActorDesc actorDesc;
	assert(convexShapeDesc.isValid());
      actorDesc.shapes.pushBack(&convexShapeDesc);
	actorDesc.body		= &bodyDesc;
	actorDesc.density		= 1.0f;
	   
      actorDesc.globalPose.t  = NxVec3(0.0f, 0.0f, 0.0f);
	assert(actorDesc.isValid());
	NxActor* actor = gScene->createActor(actorDesc);
	assert(actor);	
	return actor;
    }

    return NULL;
}

NxActor* CreateTriangleMesh(NxVec3 pos, NxVec3 siz, float dens)
{
NxVec3 boxDim(1.0f, 1.0f, 1.0f);
	// Supply hull

	 NxVec3 verts[8] = 
		{	NxVec3(-boxDim.x, -boxDim.y, -boxDim.z), 
			NxVec3(boxDim.x,  -boxDim.y, -boxDim.z), 
			NxVec3(-boxDim.x, boxDim.y,  -boxDim.z), 
			NxVec3(boxDim.x,  boxDim.y,  -boxDim.z),
			NxVec3(-boxDim.x, -boxDim.y, boxDim.z), 
			NxVec3(boxDim.x,  -boxDim.y, boxDim.z), 
			NxVec3(-boxDim.x, boxDim.y,  boxDim.z), 
			NxVec3(boxDim.x,  boxDim.y,  boxDim.z),
		};


	for (int i = 0; i < 8; i++)
	{
		verts[i].x += pos.x;
		verts[i].y += pos.y;
		verts[i].z += pos.z;
	}

	// Triangles is 12*3
	 NxU32 indices[12*3] =
 		{	1,2,3,        
			0,2,1,  
			5,4,1,    
			1,4,0,    
			1,3,5,    
			3,7,5,    
			3,2,7,    
			2,6,7,    
			2,0,6,    
			4,6,0,
			7,4,5,
			7,6,4
		};

		// Create descriptor for triangle mesh
	 NxTriangleMeshDesc* triangleMeshDesc = NULL;
    	if (!triangleMeshDesc)
	{
		triangleMeshDesc	= new NxTriangleMeshDesc();
		assert(triangleMeshDesc);
	}
	triangleMeshDesc->numVertices		= 8;
	triangleMeshDesc->pointStrideBytes		= sizeof(NxVec3);
	triangleMeshDesc->points			= verts;
	triangleMeshDesc->numTriangles		= 12;
	triangleMeshDesc->flags			= 0;
	triangleMeshDesc->triangles			= indices;
	triangleMeshDesc->triangleStrideBytes	= 3 * sizeof(NxU32);
	
	// The actor has one shape, a triangle mesh
	NxInitCooking();
	MemoryWriteBuffer buf;

	bool status = NxCookTriangleMesh(*triangleMeshDesc, buf);
	NxTriangleMesh* pMesh;
	if (status)
	{
pMesh = gPhysicsSDK->createTriangleMesh(MemoryReadBuffer(buf.data));
	}
	else
	{
		assert(false);
		pMesh = NULL;
	}
	NxCloseCooking();
	// Create TriangleMesh above code segment.

	NxTriangleMeshShapeDesc tmsd;
	tmsd.meshData		= pMesh;
	tmsd.userData			= triangleMeshDesc;
	tmsd.localPose.t		= NxVec3(0, boxDim.y, 0);
	tmsd.meshPagingMode 	= NX_MESH_PAGING_AUTO;
	
	NxActorDesc actorDesc;
	NxBodyDesc  bodyDesc;
	
	assert(tmsd.isValid());
	actorDesc.shapes.pushBack(&tmsd);
	//Dynamic triangle mesh don't be supported anymore. So body = NULL
	actorDesc.body		= NULL;		
	actorDesc.globalPose.t	= NxVec3(3.0f, 0.0f, 0.0f);

	if (pMesh)
	{
		// Save mesh in userData for drawing
		pMesh->saveToDesc(*triangleMeshDesc);
		//
		assert(actorDesc.isValid());
		NxActor *actor = gScene->createActor(actorDesc);
		assert(actor);

		return actor;
	}

	return NULL;
}
