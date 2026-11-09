// A call through -> on a data member of a std::unique_ptr or a std::shared_ptr reaches the methods of the type that it points to,
// as a call through a raw pointer does, while a call through . reaches the methods of the smart pointer itself

#include <memory>

using std::unique_ptr;

class Engine {
    int m_rpm;
public:
    // @stereotype get
    int rpm() const { return m_rpm; }

    // @stereotype set
    void start() { m_rpm = 800; }

    // @stereotype empty
    void log() const { }

    // A method with the name of a method of the smart pointer
    // @stereotype empty
    void reset() { }
};

class Car {
    std::unique_ptr<Engine> m_engine;
    std::shared_ptr<const Engine> m_spare;
    unique_ptr<Engine> m_backup;
    std::unique_ptr<Engine>* m_slot;
    boost::shared_ptr<Engine> m_shared;
    std::unique_ptr<Engine> m_front, m_rear;
    struct Wheel {
        int m_width;

        // @stereotype get
        int width() const { return m_width; }
    } *m_wheel;
public:
    // @stereotype property collaborator
    int speed() { return m_engine->rpm() / 100; }

    // @stereotype property collaborator
    int spareSpeed() { return m_spare->rpm() / 100; }

    // @stereotype property collaborator
    int backupSpeed() { return m_backup->rpm() / 100; }

    // @stereotype command collaborator
    void go() { m_engine->start(); }

    // @stereotype wrapper
    void report() { m_engine->log(); }

    // A call through . reaches the smart pointer, which is no type of the archive
    // @stereotype command collaborator
    void stop() { m_engine.reset(); }

    // A call through -> on a pointer to a smart pointer reaches the smart pointer
    // @stereotype command collaborator
    void clear() { m_slot->reset(); }

    // A smart pointer outside the standard library is no type of the archive
    // @stereotype command collaborator
    void restart() { m_shared->reset(); }

    // A later declarator of a declaration has the smart pointer of the first one
    // @stereotype property collaborator
    int rearSpeed() { return m_rear->rpm() / 100; }

    // A pointer declarator after the definition of a member type points to that type
    // @stereotype property collaborator
    int wheelWidth() { return m_wheel->width() / 2; }
};
