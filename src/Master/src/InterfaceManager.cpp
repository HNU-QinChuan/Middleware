#include "InterfaceManager.hpp"


namespace Hnu::Interface {

  InterfaceManager& InterfaceManager::getInstance() {
    static InterfaceManager instance; 
    return instance;                
  }
}