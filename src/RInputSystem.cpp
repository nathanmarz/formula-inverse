#include "Framework.h"

void RKeyboardSystem::AddKeyAction(char key, int actionId) {
    m_actions[key] = RResources::GetInputAction(actionId);
}

void RKeyboardSystem::AddKeyAction(char key, RInputAction *action) {
    m_actions[key] = action;
}


void RKeyboardSystem::ExecuteInput(int deltaTime) {
    hash_set<char> keysJustPressed;
    // Poll for events, and handle the ones we care about.
    REngine *eng = GetEngine();
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
        {
            //redirect events here
            char key = event.key.keysym.sym;
            RInputAction *action;
            switch (event.type) 
                {
                case SDL_KEYDOWN:
                    // If escape is pressed, return (and thus, quit)
//                     if (event.key.keysym.sym == SDLK_ESCAPE) {
//                         cerr<<"ESCAPE pressed... quitting..."<<endl;
//                         exit(0);
//                     }
                    m_keyStates[key] = true;
                    action = m_actions[key];
                    keysJustPressed.insert(key);
                    if(action!=NULL) action->KeyPressed(deltaTime);
                    /*
                    switch(event.key.keysym.sym) {
                    case 'w':
                        eng->GetCamera()->IncreaseSpeed();
                        break;
                    case 's':
                        eng->GetCamera()->DecreaseSpeed();
                        break;
                    case 'd':
                        eng->GetCamera()->Rotate(5);
                        
                        break;
                    case 'a':
                        eng->GetCamera()->Rotate(-5);
                        break;
                    case 'v':
                        eng->GetCamera()->SwitchViewpoint();
                        break;
                        
                    }
                    */
                    break;
                case SDL_KEYUP:
                    m_keyStates[key] = false;
                    action = m_actions[key];
                    if(action!=NULL) action->KeyUp(deltaTime);
                    break;
                case SDL_QUIT:
                    //cerr<<"Thanks for playing!"<<endl;
                    exit(0);
                }
        }

    for(hash_map<char,RInputAction *>::iterator it = m_actions.begin();it!=m_actions.end();++it) {
        if(m_keyStates[it->first]&&keysJustPressed.find(it->first)==keysJustPressed.end()) {
            if(it->second!=NULL)
                it->second->KeyDown(deltaTime);
        }
    }
}
