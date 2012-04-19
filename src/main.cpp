#include <Framework.h>
#include <SDL_main.h>

int main(int argc, char *argv[]) {
    
    bool fullScreen = argc>1;
  REngine eng("racer",fullScreen,"../",1024,768,60);
  eng.EnterGameLoop();
  return 0;
}
