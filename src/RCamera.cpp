#include "Framework.h"

RCamera::RCamera(REngine *eng, int playerId, int numPlayers) {
    m_engine = eng;
    viewAngle = 0;
    viewPos[0]=0;viewPos[1]=0;viewPos[2]=0;
    speed = 0;
    timeSinceLastMove = 0;
    onTrack = false;
    trackDistance = 1;
    freeCameraEnabled = false;

    if (playerId < 0 || playerId >= MAX_PLAYERS) {
      cerr << "RCamera() -- bad playerId" << endl;
      exit(1);
    }

    if (numPlayers < 1 || numPlayers > MAX_PLAYERS) {
      cerr << "RCamera() -- bad numPlayers" << endl;
      exit(1);
    }

    m_numPlayers = numPlayers;
    m_playerId = playerId;
}

void RCamera::GetViewRay(Vector4 &start, Vector4 &end) {
    start = m_pos;
    RPlayer *lookingAt = m_engine->GetPlayer(m_playerId);
    end = lookingAt->getPos();
}

void RCamera::SetViewport() {
  switch (m_playerId) {
  case 0:
    switch (m_numPlayers) {
    case 1:
      glViewport(0,0,(int)m_width,(int)m_height);
      glScissor(0,0,(int)m_width,(int)m_height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(GetFovy(),GetRatio(),GetNearDist(),GetFarDist());
      glMatrixMode(GL_MODELVIEW);
      break;
    case 2:
    case 3:
    case 4:
      glViewport(0,(int)m_height,(int)m_width,(int)m_height);
      glScissor(0,(int)m_height,(int)m_width,(int)m_height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(GetFovy(),GetRatio(),GetNearDist(),GetFarDist());
      glMatrixMode(GL_MODELVIEW);
      break;
    default:
      cerr << "RCamera::SetViewport -- Bad m_numPlayers" << endl;
      exit(1);
    }
    break;
  case 1:
    switch (m_numPlayers) {
    case 1:
      cerr << "RCamera::SetViewport -- "
	   << "Attempting to set viewport for a player" 
	   << " that shouldn't exit" << endl;
      exit(1);
    case 2:
      glViewport(0,0,(int)m_width,(int)m_height);
      glScissor(0,0,(int)m_width,(int)m_height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(GetFovy(),GetRatio(),GetNearDist(),GetFarDist());
      glMatrixMode(GL_MODELVIEW);
      break;
    case 3:
    case 4:
      glViewport((int)m_width,(int)m_height,(int)m_width,(int)m_height);
      glScissor((int)m_width,(int)m_height,(int)m_width,(int)m_height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(GetFovy(),GetRatio(),GetNearDist(),GetFarDist());
      glMatrixMode(GL_MODELVIEW);
      break;
    default:
      cerr << "RCamera::SetViewport -- Bad m_numPlayers" << endl;
      exit(1);      
    }
    break;
  case 2:
    switch (m_numPlayers) {
    case 1:
    case 2:
      cerr << "RCamera::SetViewport -- "
	   << "Attempting to set viewport for a player" 
	   << " that shouldn't exit" << endl;
      exit(1);
    case 3:
    case 4:
      glViewport(0,0,(int)m_width,(int)m_height);
      glScissor(0,0,(int)m_width,(int)m_height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(GetFovy(),GetRatio(),GetNearDist(),GetFarDist());
      glMatrixMode(GL_MODELVIEW);
      break;
    default:
      cerr << "RCamera::SetViewport -- Bad m_numPlayers" << endl;
      exit(1);      
    }    
    break;
  case 3:
    switch (m_numPlayers) {
    case 1:
    case 2:
    case 3:
      cerr << "RCamera::SetViewport -- "
	   << "Attempting to set viewport for a player" 
	   << " that shouldn't exit" << endl;
      exit(1);
    case 4:
      glViewport((int)m_width,0,(int)m_width,(int)m_height);
      glScissor((int)m_width,0,(int)m_width,(int)m_height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(GetFovy(),GetRatio(),GetNearDist(),GetFarDist());
      glMatrixMode(GL_MODELVIEW);
      break;
    default:
      cerr << "RCamera::SetViewport -- Bad m_numPlayers" << endl;
      exit(1);      
    }    
    break;
  default:
    cerr << "RCamera::SetViewport() -- Bad player Id" << endl;
    exit(1);
  }
}

void RCamera::Animate(int deltaTime) {
    const float UP_AMT = 1;
    const float BACK_AMT = 2.2;
    const float LOOK_AHEAD = 6;
    
    glLoadIdentity();

    RPlayer *player = m_engine->GetPlayer(m_playerId);
    Vector4 desiredUp = Normalize3(player->GetUp());
    
    Vector4 playAhead = player->GetAhead();
    Vector4 dirNormal = CrossProduct(playAhead,desiredUp);
    playAhead = Normalize3(CrossProduct(desiredUp,dirNormal));
    
    Vector4 desiredPos = player->GetPosition()+desiredUp*UP_AMT-playAhead*BACK_AMT;

    Vector4 desiredAhead = Normalize3(player->GetPosition()+playAhead*LOOK_AHEAD-desiredPos);
    
    
    float distance = Length3(desiredPos - m_pos);

    m_pos = desiredPos;
    m_ahead = desiredAhead;
    m_up = CrossProduct(CrossProduct(m_ahead,desiredUp),m_ahead);
    Vector4 aa = m_pos+m_ahead;
    if(!freeCameraEnabled)
        gluLookAt(m_pos[0],m_pos[1],m_pos[2],aa[0],aa[1],aa[2],m_up[0],m_up[1],m_up[2]);

    //m_ahead = playAhead;
    // NOTE:
    // m_ahead and m_up must be normal to each other
    // AND correspond exactly to depth and height.
    // View Frustum culling finds the the vertices
    // of the view frustum by moving in these directions,
    // so if they are not exactly straight ahead and straight up
    // there will be a problem.
    //m_ahead = Normalize3(player->GetAhead());
    //m_ahead = Normalize3(desiredAhead);
    //m_up = desiredUp;
    //m_up = Normalize3(player->GetUp());


    //Vector4 pointAhead = desiredPos+desiredAhead;
    //Vector4 pointAhead = m_pos+m_ahead;
    
    //gluLookAt(m_pos[0],m_pos[1],m_pos[2],pointAhead[0],pointAhead[1],pointAhead[2],m_up[0],m_up[1],m_up[2]);
    //glLoadIdentity();
    timeSinceLastMove +=deltaTime;
    //if(timeSinceLastMove>100) {
    timeSinceLastMove = 0;
    viewPos[0]+=-speed*sin(viewAngle*PI/180);;
    viewPos[2]+=speed*cos(viewAngle*PI/180);
    
    
    if (freeCameraEnabled) {
      if(onTrack) m_engine->GetWorld()->getTrack()->Viewpoint(trackDistance,12);
      else {
	glRotatef(viewAngle,0,1,0);
	glTranslatef(viewPos[0],viewPos[1],viewPos[2]);
      }
    }

    /*
    Vector4 pos = player->GetPosition()+.2*desiredUp;
    Vector4 a = pos+playAhead;
    
    m_ahead = Normalize3(playAhead);
    
    Vector4 upNormalizer = CrossProduct(m_ahead,desiredUp);
    m_up = Normalize3(CrossProduct(upNormalizer,m_ahead));
    
    if (!freeCameraEnabled)
      gluLookAt(pos[0],pos[1],pos[2],a[0],a[1],a[2],m_up[0],m_up[1],m_up[2]);
    
    m_pos = pos;
    */
//     cout << "Position: "; PrintVector(m_pos); cout << endl;
//     cout << "Ahead: "; PrintVector(m_ahead); cout << endl;
//     cout << "Up: "; PrintVector(m_up); cout << endl;
}


void RCamera::SwitchViewpoint() { onTrack = !onTrack; speed = 0;}
