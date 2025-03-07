


namespace Hnu::Interface {
  class InterfaceManager {
  public:
    InterfaceManager(const InterfaceManager&) = delete;
    InterfaceManager& operator=(const InterfaceManager&) = delete;
    InterfaceManager(InterfaceManager&&) = delete;
    InterfaceManager& operator=(InterfaceManager&&) = delete;
    static InterfaceManager& getInstance();
  private:
    InterfaceManager()=default;

  };
}