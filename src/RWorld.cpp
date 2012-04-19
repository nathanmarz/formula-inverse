#include "Framework.h"
#include "RFontManager.h"
#include <fstream>
#include "RBillBoard.h"
#include "RParticle.h"

RWorld::RWorld(REngine *eng, const string &fileToLoad) {
    m_engine = eng;
    init(fileToLoad);
    
    //GLuint billboard = RResources::GetTexture(7);
    //rbb = new RWorldBillBoard(Vector4(0,3,10),5,5,billboard,VIEWPOINT, true);
    //ps = new RParticleSystem(Vector4(0,3,10),Vector4(0,1,0),Vector4(1,0,0),200, texture, 40, .1);
}


RWorld::~RWorld() {
    delete m_track;
    delete m_bg;
    delete m_gameLogic;
    delete m_fontManager;
    delete m_tree;
    for(list<RScenery *>::iterator it = m_scenery.begin();it!=m_scenery.end();it++) {
        delete *it;
    }
    for(list<RObject *>::iterator it = m_dynScenery.begin();it!=m_dynScenery.end();it++) {
        delete *it;
    }
}

void RWorld::AddDynamicEffect(RObject *ro) {
    m_dynScenery.push_back(ro);
}


void RWorld::Prepare() {
    
}

void RWorld::BuildTree(list<RObject *> &static_objs) {
  // Create a list
    m_tree = new RBSPtree(&static_objs,&m_dynScenery);
}

void RWorld::Animate(int deltaTime) {
    list<RObject *>::iterator it = m_dynScenery.begin();
    //cerr<<"size:"<<m_dynScenery.size()<<endl;
    while (it!=m_dynScenery.end()) {
        if((*it)->isDead()) {
            delete *it;
            //cerr<<"erased"<<endl;
            it = m_dynScenery.erase(it);
        } else {
            (*it)->Animate(deltaTime);
            ++it;
        }

    }
    //cerr<<"size:"<<m_dynScenery.size()<<endl;
    
    for(list<RScenery *>::iterator it2 = m_scenery.begin();it2!=m_scenery.end();++it2) {
        (*it2)->Animate(deltaTime);
    }
    //    ps->Animate(deltaTime);
}

void RWorld::SetLights() {
   for(int i=0;i<m_lights.size();i++) {
        GLfloat *pos = m_lights[i].position;
        unsigned int lightToUse = GetLightConstant(i);
        glLightfv(lightToUse, GL_POSITION, pos);
    }
}

void RWorld::Draw(RCamera *viewer) {
    
    SetLights();
    //CheckOpenGLErrors(31);  
    m_bg->Render(viewer);
    //CheckOpenGLErrors(32);  
    m_tree->RBSPtreeRender(viewer);
    //CheckOpenGLErrors(33);  
    //rbb->Render(viewer);
    //ps->Render(viewer);
    //    m_engine->DrawPlayerDisplay(viewer->GetPlayerId());
    //m_scenery.front()->RenderBV();
    //  m_scenery.back()->RenderBV();
    //assert(m_scenery.front()!=m_scenery.back());
    // for(list<RScenery *>::iterator it = m_scenery.begin();it!=m_scenery.end();++it) {
       //    (*it)->Render(viewer);
    //}
    // Write some text to the screen..
    //SDL_Color color;
    //color.r = 255;
    //color.g = 255;
    //color.b = 255;
    //float y = (viewer->GetHeight()*0.9);
    //float x = (viewer->GetWidth()*0.1);
    //m_fontManager->SetSize((int)(20.0/600.0*viewer->GetHeight()));
    //m_fontManager->WriteText("I hope you don't throw up", color, x,y);


    
    //    m_engine->GetPlayer()->DrawBounds();
    //track->RenderAll(viewer);
    //    glDisable(GL_LIGHTING);
    
    //glPushMatrix();
    //glTranslatef(50,0,80);
    //make him face the track
    //glTranslatef(54,-25,-5);
    //glRotatef(180.0f,0,1,0);
    //glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
    //glColor3f(1.0, 1.0, 1.0);

    //myModel.Animate(0,39,.2);
    //gunModel.Animate(0,39,.2);
    //    myModel.RenderFrame(i);
    //cerr<<"rendered"<<endl;
    //gunModel.RenderFrame(i);
    //glPopMatrix();
    //glEnable(GL_LIGHTING);
}


void RWorld::LoadLightDataLine(string &lineNonSplit, LightInfo &li) {
    vector<string> line = SplitString(lineNonSplit);
    
    Vector4 v(atof(line[1].c_str()),atof(line[2].c_str()),atof(line[3].c_str()),atof(line[4].c_str()));
    if(line[0]=="values") {
        li.ambient_diffuse[0] = v[0];
        li.ambient_diffuse[1] = v[1];
        li.ambient_diffuse[2] = v[2];
        li.ambient_diffuse[3] = v[3];
    } else if(line[0]=="position") {
        li.position[0] = v[0];
        li.position[1] = v[1];
        li.position[2] = v[2];
        li.position[3] = v[3];
    } else {
        cerr<<"improperly formatted light info"<<endl;
        exit(1);
    }
}

void RWorld::LoadLightData(vector<string> &data) {
    if(data.size()<3||data.size()%2==0) {
        cerr<<"not enough data specified for light (ambient_diffuse, position)"<<endl;
        exit(1);
    }
    for(int i=1;i<data.size();i+=2) {
        LightInfo li;
        LoadLightDataLine(data[i],li);
        LoadLightDataLine(data[i+1],li);
        m_lights.push_back(li);
    }
    
}

void RWorld::LoadFontManager(vector<string> &data) {
    if(data.size()!=3) {
        cerr<<"not enough data specified for font manager (path, size)"<<endl;
        exit(1);
    }
    string path = m_engine->getFontsDir()+data[1];
    int size = atoi(data[2].c_str());
    m_fontManager = new RFontManager(path.c_str(),size);
}

void RWorld::LoadTrackAndGameLogic(vector<string> &data) {
    int numLaps = 1;
    bool cyclic = false;
    vector<string> header = SplitString(data[0]);
    if(header.size()<3) {
        cerr<<"bad track header"<<endl;
        exit(1);
    }
    int checkPoints = atoi(header[1].c_str());
    int numBoosts = atoi(header[2].c_str());
    
    if(header.size()==3) {
        
    } else if(header.size()==5) {
        cyclic = true;
        numLaps = atoi(header[4].c_str());
    } else {
        cerr<<"bad track header"<<endl;
        exit(1);
    }
    vector<RPieceShell *> pieces;
    int widthLOD, lengthLOD;
    GLuint tex;
    for(int i=1;i<data.size();i++) {
        vector<string> line = SplitString(data[i]);
        string key = line[0];
        if(key=="pieces") {
            for(int j=1;j<line.size();j++) {
                pieces.push_back(RResources::GetPieceShell(atoi(line[j].c_str())));
            }
        } else if(key=="widthDivide") {
            assert(line.size()==2);
            widthLOD = atoi(line[1].c_str());
        } else if(key=="lengthDivide") {
            assert(line.size()==2);
            lengthLOD = atoi(line[1].c_str());
        } else if(key=="texture") {
            assert(line.size()==2);
            tex = RResources::GetTexture(atoi(line[1].c_str()));
        } else {
            cerr<<"Unknown line in track definition"<<endl;
            exit(1);
        }
    }
   
    m_track = new RTrack(pieces,widthLOD,lengthLOD,tex,cyclic);
    m_gameLogic = m_track->CreateGameLogic(checkPoints,numLaps,numBoosts);
    
}

Vector4 ReadVectorFromNamedArray(vector<string> &line) {
    Vector4 v;
    if(line.size()==5) {
        v = Vector4(atof(line[1].c_str()),atof(line[2].c_str()),atof(line[3].c_str()),atof(line[4].c_str()));
    } else if(line.size()==4) {
        v = Vector4(atof(line[1].c_str()),atof(line[2].c_str()),atof(line[3].c_str()));
    } else {
        cerr<<"improper size of vector array"<<endl;
        exit(1);
    }
    return v;
}

void RWorld::LoadScenery(vector<string> &data) {
    int startIndex = 1;
    int endIndex;
    for(int i=startIndex+1;;i++) {
        if(i==data.size()-1||data[i]=="endobject") {
            endIndex = i-1;
            Vector4 pos, ahead, up;
            RScenery *toInsert;
            for(int j=startIndex;j<=endIndex;j++) {
                vector<string> line = SplitString(data[j]);
                if(line[0]=="pos") {
                    pos = ReadVectorFromNamedArray(line);
                } else if(line[0]=="ahead") {
                    ahead = ReadVectorFromNamedArray(line);
                } else if(line[0]=="up") {
                    up = ReadVectorFromNamedArray(line);
                } else {
                    if(j!=endIndex) {
                        cerr<<"error: scenery type definition before coordinate frame definition"<<endl;
                        exit(1);
                    }
                    if(line[0]=="md2") {
                        float interpolate = atof(line[1].c_str());
                        string modelPath = m_engine->getModelsDir()+line[2];
                        string texPath = m_engine->getModelsDir()+line[3];
                        Matrix4 ident;
                        ident.Identity();
                        //                        toInsert = new RMD2Model(false,pos,ahead,up,ident,interpolate,modelPath.c_str(),texPath.c_str());
                    } else if(line[0]=="sign"){
                        vector<SignFlipper> textures;
                        float w = atof(line[1].c_str());
                        float h = atof(line[2].c_str());
                        for(int q=3;q<line.size();q+=2) {
                            SignFlipper si(atoi(line[q].c_str()),RResources::GetTexture(atoi(line[q+1].c_str())));
                            textures.push_back(si);
                            toInsert = new RSign(pos,up,ahead,w,h,textures);
                        }
                        
                    } else {
                        cerr<<"error: unknown scenery type: " + line[0] <<endl;
                        exit(1);
                    }
                }
            }
            //toInsert->finalize();
            m_scenery.push_front(toInsert);
            startIndex = i+1;
        }
        if(i==data.size()-1) break;
    }
}


void RWorld::LoadDome(vector<string> &data) {
    vector<string> header = SplitString(data[0]);
    float radius = atof(header[1].c_str());
    int texId = atoi(header[2].c_str());
    m_bg = new REnclosingDome(RResources::GetTexture(texId),radius);
}

void RWorld::LoadCube(vector<string> &data) {
    if(data.size()!=7) {
        cerr<<"incorrect format for dome definition"<<endl;
        exit(1);
    }
    vector<string> header = SplitString(data[0]);
    float radius = atof(header[1].c_str());
    bool doClamp = false;
    if(header.size()>2) doClamp = true;
    vector<EnclosingCubeInfo> sides;
    for(int i=1;i<7;i++) {
        EnclosingCubeInfo side;
        vector<string> texInfo = SplitString(data[i]);
        side.textureScale = atoi(texInfo[1].c_str());
        side.texture = RResources::GetTexture(atoi(texInfo[2].c_str()));
        sides.push_back(side);
    }
    m_bg = new REnclosingCube(sides,radius,doClamp);
}


void RWorld::LoadFogInfo(vector<string> &data,FogInfo &ret) {
    if(data.size()!=1) {
        cerr<<"Improperly formatted fog definition."<<endl;
        exit(1);
    }
    vector<string> fs = SplitString(data[0]);
    if(fs.size()!=6) {
        cerr<<"Improper # of values in fog definition."<<endl;
        exit(1);
    }
    ret.fogDensity = atof(fs[1].c_str());
    ret.fogVals[0] = atof(fs[2].c_str());
    ret.fogVals[1] = atof(fs[3].c_str());
    ret.fogVals[2] = atof(fs[4].c_str());
    ret.fogVals[3] = atof(fs[5].c_str());
}

void RWorld::init(const string &fileName) {
    string filePath = m_engine->getLevelsDir()+fileName;
    ifstream levelFile;
    levelFile.open(filePath.c_str());
    if(!levelFile) {
        cerr<<"Could not open level "+fileName<<endl;
        exit(1);
    }

    bool fogEnabled = false;
    FogInfo fogInfo;
    
    while(true) {
        vector<string> data;
        if(!ReadFileUntilTerminateLine(levelFile,"end",data)) break;
        string key = data[0];
        if(key.find("light")!=string::npos) {
            LoadLightData(data);
        } else if(key.find("fontManager")!=string::npos) {
            LoadFontManager(data);
        } else if(key.find("track")!=string::npos) {
            LoadTrackAndGameLogic(data);
        } else if(key.find("scenery")!=string::npos) {
            LoadScenery(data);
        } else if(key.find("dome")!=string::npos) {
            LoadDome(data);
        } else if(key.find("cube")!=string::npos) {
            LoadCube(data);
        } else if(key.find("fog")!=string::npos) {
            fogEnabled = true;
            LoadFogInfo(data,fogInfo);
        } else {
            cerr<<"Invalid block for level definition"<<endl;
            exit(1);
        }
    }
    //CheckOpenGLErrors(-3);
    glClearColor(0.0,0.0,0.0,1);
    glShadeModel(GL_SMOOTH);
    DisableAllLights(); //only enable ones we're going to use
    
    for(int i=0;i<m_lights.size();i++) {
        unsigned int light = GetLightConstant(i);
        glLightfv(light, GL_DIFFUSE, m_lights[i].ambient_diffuse);
        glLightfv(light, GL_AMBIENT, m_lights[i].ambient_diffuse);
        glEnable(light);
    }
    //glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
    
    //CheckOpenGLErrors(-2);
    //glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
    //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);

    //glEnable(GL_FOG);
    //GLfloat fogVal[] = {.1,.1,.4,0};
    //glFogfv(GL_FOG_COLOR,fogVal);
    //glFogf(GL_FOG_DENSITY,.04);

    if(fogEnabled) {
        glEnable(GL_FOG);
        glFogfv(GL_FOG_COLOR,fogInfo.fogVals);
        glFogf(GL_FOG_DENSITY,fogInfo.fogDensity);
    } else {
        glDisable(GL_FOG);
    }
    //CheckOpenGLErrors(-1);
    //vector<SignFlipper> s_textures;
    //s_textures.push_back(SignFlipper(3000,RResources::GetTexture(8)));
    //s_textures.push_back(SignFlipper(3000,RResources::GetTexture(7)));
    //RSign *rs = new RSign(Vector4(0,5,0),Vector4(0,1,0),Vector4(0,0,-1),4,4,s_textures);
    //m_scenery.push_front(rs);

    m_scenery.push_front(new RFinishLine(m_track,m_gameLogic->GetFinishI()));
    
    list<RObject *> objects = m_track->GetListOfPieces();
    //add static scenery to bsp tree
    objects.insert(objects.begin(),m_scenery.begin(),m_scenery.end());
    

    
    //CheckOpenGLErrors(0);
    BuildTree(objects);
    //cerr<<"tree built"<<endl;
    //exit(1);
}


/*
void RWorld::init(const string &fileName) {
    //GLfloat mat_specular[] = { .5,.5,.5,.3 };
    //GLfloat mat_shininess[] = {100.0};
    //GLfloat mat_ambient[] = {.7,.2,.3,.5 };
 
    GLfloat white_light[] = {1.0,1.0,1.0,1.0 };
    
    glClearColor(0.0,0.0,0.0,.5);
    glShadeModel(GL_SMOOTH);
    
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_AMBIENT,white_light);
    
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    //glBlendFunc(GL_ONE, GL_ONE);
    //glEnable(GL_BLEND);
    //cerr<<'a'<<endl;
    RResources::AddTexture(2,"../textures/wlbk007.jpg");
    RResources::AddTexture(3,"../textures/SkyDomeImage.jpg");
    //cerr<<"b"<<endl;

    RPieceShell rps1(4,16), rps2(4,8);
    
    rps2.push_back(REdgeShell(Vector4(15,0,10),Vector4(10,0,10)));
    rps2.push_back(REdgeShell(Vector4(15,-2,5),Vector4(10,-2,5)));
    vector<Vector4> ps;
    ps.push_back(Vector4(12.5,2.5,0));
    ps.push_back(Vector4(15,-0,0));
    ps.push_back(Vector4(12.5,0,0));    
    ps.push_back(Vector4(10,-0,0));
    //ps.push_back(Vector4(12.5,2.5,0));
    rps2.push_back(REdgeShell(ps,true));
    vector<Vector4> ps2;
    ps2.push_back(Vector4(12.5,2.5,-10));
    ps2.push_back(Vector4(15,-0,-10));
    ps2.push_back(Vector4(12.5,0,-10));    
    ps2.push_back(Vector4(10,-0,-10));
    //    ps2.push_back(Vector4(12.5,2.5,-10));
    rps2.push_back(REdgeShell(ps2,true));
    rps2.push_back(REdgeShell(Vector4(15,0,-12),Vector4(10,0,-12)));
    rps2.push_back(REdgeShell(Vector4(15,0,-16),Vector4(10,0,-16)));
    

    //   /*
    rps1.push_back(REdgeShell(Vector4(0,2,0),Vector4(10,0,0)));
    rps1.push_back(REdgeShell(Vector4(0,2,20),Vector4(10,0,10)));
    rps1.push_back(REdgeShell(Vector4(40,2,20),Vector4(40,0,10)));
    // */
    /*
    rps2.push_back(REdgeShell(Vector4(5,0,-25),Vector4(0,0,-25)));
    rps2.push_back(REdgeShell(Vector4(5,0,-20),Vector4(0,2,-20)));
    rps2.push_back(REdgeShell(Vector4(3,1,-15),Vector4(3,4,-15)));
    rps2.push_back(REdgeShell(Vector4(0,0,-10),Vector4(5,2,-10)));
    rps2.push_back(REdgeShell(Vector4(0,0,-5),Vector4(5,0,-5)));

    //
    rps2.push_back(REdgeShell(Vector4(0,0,0),Vector4(5,0,0)));
    rps2.push_back(REdgeShell(Vector4(0,0,8),Vector4(5,0,8)));
    rps2.push_back(REdgeShell(Vector4(0,1,10),Vector4(5,1,10)));
    rps2.push_back(REdgeShell(Vector4(1,5,12),Vector4(6,5,12)));    
    rps2.push_back(REdgeShell(Vector4(3,8,9),Vector4(8,8,9)));
    rps2.push_back(REdgeShell(Vector4(6,4,7),Vector4(11,4,7)));
    rps2.push_back(REdgeShell(Vector4(7,0,12),Vector4(12,0,12)));
    rps2.push_back(REdgeShell(Vector4(8,0,15),Vector4(13,0,15)));
    
    
    rps1.finalize();
    rps2.finalize();
    //rps3.finalize();
		   
    vector<RPieceShell *> pieces;
    pieces.push_back(&rps1);
    pieces.push_back(&rps2);
    pieces.push_back(&rps1);
    pieces.push_back(&rps1);
 
 
    track = new RTrack(pieces,25,30,RResources::GetTexture(2),true);
    //  cerr<<"loading 1..."<<endl;
    m_gameLogic = track->CreateGameLogic(5,3);

    
    //    myModel = new RMD2Model(Vector4(54,-25,-5),Vector4(0,0,1),Vector4(0,1,0),.2,"./Models/Ogro/Tris.MD2", "./Models/Ogro/Ogrobase.pcx");
    //gunModel = new RMD2Model(Vector4(54,-25,-5),Vector4(0,0,1),Vector4(0,1,0),.2,"./Models/Ogro/Weapon.md2", "./Models/Ogro/Weapon.pcx");
    myModel = new RMD2Model(Vector4(54,-10,-5),Vector4(-1,0,-1),Vector4(0,1,0),.2,"./Models/Car/tris.md2","./Models/Car/astro.pcx");
    myModel->finalize();
    //    gunModel->finalize();
    m_testScenery = myModel;

    m_dome = new REnclosingDome(Vector4(0,0,0),500,RResources::GetTexture(3));

    //cerr<<"hello"<<endl;
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);

    list<RObject *> objects = track->GetListOfPieces();
    objects.push_front(myModel);
    //objects.push_front(gunModel);
    
    BuildTree(objects);
}

*/
